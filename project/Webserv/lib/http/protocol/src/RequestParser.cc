#include "http/core/RequestParser.hpp"
#include "FileUtils.hpp"
#include "logger.hpp"
#include <algorithm>
#include <expected>
#include <magic_enum/magic_enum.hpp>
#include <sstream>
#include <string_view>

namespace http {

namespace {
    const std::string_view CRLF = "\r\n";
    const std::string_view DOUBLE_CRLF = "\r\n\r\n";

    bool iequal(std::string_view a, std::string_view b)
    {
        return std::ranges::equal(a, b, [](char x, char y) {
            return std::tolower(static_cast<unsigned char>(x)) == std::tolower(static_cast<unsigned char>(y));
        });
    }

} // namespace

RequestParser::RequestParser(size_t max_body_size)
    : _step(Step::RequestLine)
    , _max_body_size(max_body_size)

{
    reset();
}

void RequestParser::reset()
{
    _step = Step::RequestLine;
    _buffer.clear();
    _request = {};
    _client_wants_keep_alive = false;
    _chunk_size_remaining = 0;
}

auto RequestParser::getRequest() const -> const Request&
{
    return _request;
}

auto RequestParser::parse(std::string_view data) -> std::expected<IRequestParser::State, StatusCode>
{
    _buffer.append(data);

    // **只要我们还在取得进展，就持续驱动状态机**
    bool progress_made = true;
    while (progress_made) {
        progress_made = false;

        // 记录进入循环前的 buffer 大小
        size_t buffer_size_before = _buffer.size();

        std::expected<ParseResult, StatusCode> result;

        switch (_step) {
        case Step::RequestLine:
            result = parseRequestLine();
            if (result && *result == ParseResult::Success)
                _step = Step::Headers;
            break;

        case Step::Headers:
            result = parseHeaders();
            if (result && *result == ParseResult::Success) {
                if (_request.headers.contains("Transfer-Encoding"))
                    _step = Step::ChunkedBody;
                else if (_request.headers.contains("Content-Length"))
                    _step = Step::Body;
                else
                    _step = Step::Completed;
            }
            break;

        case Step::Body:
            result = parseBody();
            if (result && *result == ParseResult::Success)
                _step = Step::Completed;
            break;

        case Step::ChunkedBody:
            result = parseChunkedBody();
            if (result && *result == ParseResult::Success)
                _step = Step::Completed;
            break;

        case Step::Completed:
            return IRequestParser::State::Completed;
        }

        // --- 检查结果 ---
        if (!result) {
            return std::unexpected(result.error()); // 发生错误，立刻返回
        }

        // **如果子解析器消耗了数据，就说明取得了进展，主循环应该继续**
        if (_buffer.size() < buffer_size_before) {
            progress_made = true;
        }

        // 如果已经完成，也算取得了进展（以便下一次循环进入 Completed case）
        if (_step == Step::Completed) {
            progress_made = true;
        }
    }

    // 如果循环结束 (没有取得任何进展)，则说明我们需要更多数据
    if (_step == Step::Completed) {
        return IRequestParser::State::Completed;
    }
    return IRequestParser::State::Parsing;
}

// --- 子步骤函数 ---

auto RequestParser::parseRequestLine() -> std::expected<ParseResult, StatusCode>
{
    const auto pos = _buffer.find(CRLF);
    if (pos == std::string::npos) {
        return ParseResult::Incomplete; // 数据不足
    }

    std::stringstream ss(_buffer.substr(0, pos));
    _buffer.erase(0, pos + CRLF.length());

    std::string method_str;
    ss >> method_str >> _request.uri >> _request.version;

    if (ss.fail() || !ss.eof()) {
        return std::unexpected(StatusCode::BadRequest);
    }
    auto normalized_uri_or_error = utils::normalizeUriPath(_request.uri);
    if (!normalized_uri_or_error) {
        // 如果规范化失败（例如检测到路径遍历），返回 400 Bad Request
        return std::unexpected(StatusCode::BadRequest);
    }
    // 将请求中的 URI 替换为干净的版本
    _request.uri = std::move(*normalized_uri_or_error);

    const auto method_opt = magic_enum::enum_cast<http::Method>(method_str);
    if (!method_opt) {
        return std::unexpected(StatusCode::NotImplemented);
    }
    _request.method = *method_opt;

    if (!_request.version.starts_with("HTTP/")) {
        return std::unexpected(StatusCode::HTTPVersionNotSupported);
    }

    return ParseResult::Success; // 成功
}

auto RequestParser::parseHeaders() -> std::expected<ParseResult, StatusCode>
{
    if (_buffer.starts_with(CRLF)) {
        _buffer.erase(0, CRLF.length());
        return ParseResult::Success;
    }

    const auto endOfHeadersPos = _buffer.find(DOUBLE_CRLF);
    if (endOfHeadersPos == std::string::npos) {
        return ParseResult::Incomplete; // 数据不足
    }

    const std::string headersPart = _buffer.substr(0, endOfHeadersPos);
    _buffer.erase(0, endOfHeadersPos + DOUBLE_CRLF.length());

    std::stringstream ss(headersPart);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }

        const auto colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            return std::unexpected(StatusCode::BadRequest);
        }

        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);

        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        _request.headers[key] = value;
    }
    if (auto it = _request.headers.find("Transfer-Encoding"); it != _request.headers.end()) {
        if (iequal(it->second, "chunked")) {
            // 如果是 chunked，我们准备进入 ChunkedBody 解析
            // **注意：我们不再在这里返回 NotImplemented！**
            // 我们将在主 `parse` 循环的 `if (result && *result == ParseResult::Success)`
            // 之后，根据这个头来切换状态。
        } else {
            // 我们只支持 chunked，不支持其他 Transfer-Encoding
            return std::unexpected(StatusCode::NotImplemented);
        }
    }

    // 检查 Content-Length 值的有效性
    if (auto it = _request.headers.find("Content-Length"); it != _request.headers.end()) {
        try {
            size_t content_length = std::stoull(it->second); // 使用 stoull 以防溢出
            if (content_length > _max_body_size) {
                return std::unexpected(StatusCode::PayloadTooLarge);
            }
        } catch (const std::exception&) {
            // stoull 失败，说明 Content-Length 的值不是有效数字
            return std::unexpected(StatusCode::BadRequest);
        }
    }

    const auto& req = _request; // for brevity
    _client_wants_keep_alive = true;
    if (req.version == "HTTP/1.0") {
        _client_wants_keep_alive = false; // HTTP/1.0 默认
    }
    if (auto it = req.headers.find("Connection"); it != req.headers.end()) {
        LOG_DEBUG("User specified : Connection header found: {}", it->second);
        if (iequal(it->second, "close")) {
            _client_wants_keep_alive = false;
        } else if (iequal(it->second, "keep-alive")) {
            _client_wants_keep_alive = true;
        }
    }
    LOG_INFO("header parsed: {}", req.headers);
    return ParseResult::Success; // 成功
}

auto RequestParser::parseBody() -> std::expected<ParseResult, StatusCode>
{
    const auto it = _request.headers.find("Content-Length");
    if (it == _request.headers.end()) {
        // 理论上不应该发生，因为状态机逻辑会阻止无 Content-Length 的请求进入此步骤
        return std::unexpected(StatusCode::BadRequest);
    }

    const size_t contentLength = std::stoul(it->second);
    if (_buffer.size() < contentLength) {
        return ParseResult::Incomplete; // 数据不足
    }

    _request.body.assign(_buffer.begin(), _buffer.begin() + contentLength);
    _buffer.erase(0, contentLength);

    return ParseResult::Success; // 成功
}
auto RequestParser::parseChunkedBody() -> std::expected<ParseResult, StatusCode>
{
    LOG_INFO("Parsing chunked body, current buffer size: {}", _buffer.size());
    // Phase A: need a chunk-size line if we have no remaining bytes to read
    if (_chunk_size_remaining == 0) {
        while (_buffer.starts_with(CRLF)) {
            _buffer.erase(0, CRLF.length());
        }
        if (_buffer.empty())
            return ParseResult::Incomplete;

        auto pos = _buffer.find(CRLF);
        if (pos == std::string::npos)
            return ParseResult::Incomplete;

        std::string size_line = std::string(_buffer.substr(0, pos));
        _buffer.erase(0, pos + CRLF.length());

        auto semi = size_line.find(';');
        if (semi != std::string::npos)
            size_line = size_line.substr(0, semi);
        auto l = size_line.find_first_not_of(" \t");
        auto r = size_line.find_last_not_of(" \t");
        if (l == std::string::npos)
            size_line.clear();
        else
            size_line = size_line.substr(l, r - l + 1);

        size_t chunk_size = 0;
        try {
            chunk_size = std::stoul(size_line, nullptr, 16);
        } catch (...) {
            return std::unexpected(StatusCode::BadRequest);
        }

        _chunk_size_remaining = chunk_size;

        if (_chunk_size_remaining == 0) {
            if (_buffer.starts_with(CRLF)) {
                _buffer.erase(0, CRLF.length());
                return ParseResult::Success;
            }
            auto trailer_end = _buffer.find(DOUBLE_CRLF);
            if (trailer_end != std::string::npos) {
                _buffer.erase(0, trailer_end + DOUBLE_CRLF.length());
                return ParseResult::Success;
            }
            return ParseResult::Incomplete;
        }
        return ParseResult::Incomplete;
    }

    // Phase B: reading chunk-data
    // --- NEW: before consuming chunk_size bytes, check if there's an *early* CRLF within the
    // declared chunk region that is followed by a complete and valid chunk-size line.
    // If so, the declared chunk_size lied (sender ended the chunk earlier) => BadRequest.

    // We will scan up to min(_chunk_size_remaining, _buffer.size()) for CRLF positions.
    size_t scan_limit = std::min(_chunk_size_remaining, _buffer.size());
    auto scan_pos = _buffer.find(CRLF);
    while (scan_pos != std::string::npos && scan_pos < scan_limit) {
        // potential CRLF found at scan_pos which is before declared end.
        // See if what follows is a complete line terminated by CRLF.
        size_t after = scan_pos + CRLF.size();
        auto next_crlf = _buffer.find(CRLF, after);
        if (next_crlf == std::string::npos) {
            // next line incomplete -> can't decide yet; break out of scan loop
            break;
        }
        // extract candidate size token
        std::string potential_size = std::string(_buffer.substr(after, next_crlf - after));
        auto semi = potential_size.find(';');
        if (semi != std::string::npos)
            potential_size = potential_size.substr(0, semi);
        auto ll = potential_size.find_first_not_of(" \t");
        auto rr = potential_size.find_last_not_of(" \t");
        if (ll == std::string::npos)
            potential_size.clear();
        else
            potential_size = potential_size.substr(ll, rr - ll + 1);

        bool all_hex = !potential_size.empty() && std::all_of(potential_size.begin(), potential_size.end(), [](unsigned char c) {
            return std::isxdigit(c);
        });

        if (all_hex) {
            // Found CRLF before the declared chunk end, and what follows looks like a chunk-size line.
            // This indicates the sender ended the chunk early -> protocol error.
            return std::unexpected(StatusCode::BadRequest);
        }
        // else it's a CRLF inside data; look for next CRLF within the scan range
        scan_pos = _buffer.find(CRLF, scan_pos + CRLF.size());
    }

    // If not enough data to reach declared chunk size, consume partial and wait
    if (_buffer.size() < _chunk_size_remaining) {
        _request.body.insert(_request.body.end(), _buffer.begin(), _buffer.end());
        _chunk_size_remaining -= _buffer.size();
        _buffer.clear();

        if (_request.body.size() > _max_body_size) {
            return std::unexpected(StatusCode::PayloadTooLarge);
        }
        return ParseResult::Incomplete;
    }

    // We have at least chunk_size bytes available. Read exactly that many.
    _request.body.insert(_request.body.end(), _buffer.begin(), _buffer.begin() + _chunk_size_remaining);
    _buffer.erase(0, _chunk_size_remaining);
    _chunk_size_remaining = 0;

    if (_request.body.size() > _max_body_size) {
        return std::unexpected(StatusCode::PayloadTooLarge);
    }

    // After chunk-data, there MUST be a CRLF. If not enough bytes available, wait.
    if (_buffer.size() < CRLF.size())
        return ParseResult::Incomplete;
    if (!_buffer.starts_with(CRLF))
        return std::unexpected(StatusCode::BadRequest);

    _buffer.erase(0, CRLF.size());
    return ParseResult::Incomplete;
}

bool RequestParser::clientWantsKeepAlive() const
{
    return _client_wants_keep_alive;
}

} // namespace http