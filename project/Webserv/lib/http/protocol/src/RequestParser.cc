#include "http/core/RequestParser.hpp"
#include "FileUtils.hpp"
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
}

auto RequestParser::getRequest() const -> const Request&
{
    return _request;
}

auto RequestParser::parse(std::string_view data) -> std::expected<IRequestParser::State, StatusCode>
{
    _buffer.append(data);

    while (true) {
        std::expected<ParseResult, StatusCode> result;

        switch (_step) {
        case Step::RequestLine:
            result = parseRequestLine();
            if (result && *result == ParseResult::Success) {
                _step = Step::Headers;
            }
            break;

        case Step::Headers:
            result = parseHeaders();
            if (result && *result == ParseResult::Success) {
                auto it = _request.headers.find("Content-Length");
                if (it != _request.headers.end() && std::stoul(it->second) > 0) {
                    _step = Step::Body;
                } else {
                    _step = Step::Completed;
                }
            }
            break;

        case Step::Body:
            result = parseBody();
            if (result && *result == ParseResult::Success) {
                _step = Step::Completed;
            }
            break;

        case Step::Completed:
            return IRequestParser::State::Completed;
        }

        if (!result) {
            return std::unexpected(result.error());
        }
        if (*result == ParseResult::Incomplete) {
            return IRequestParser::State::Parsing;
        }
    }
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

    // 还可以检查 Transfer-Encoding: chunked 的情况，如果不支持，也在这里拒绝
    if (_request.headers.contains("Transfer-Encoding")) {
        // 假设我们还不支持 chunked，这是一个很好的返回点
        return std::unexpected(StatusCode::NotImplemented);
    }

    const auto& req = _request; // for brevity
    if (auto it = req.headers.find("Connection"); it != req.headers.end()) {
        if (iequal(it->second, "close")) {
            _client_wants_keep_alive = false;
        } else if (iequal(it->second, "keep-alive")) {
            _client_wants_keep_alive = true;
        }
    } else if (req.version == "HTTP/1.1") {
        _client_wants_keep_alive = true; // HTTP/1.1 默认
    } else {
        _client_wants_keep_alive = false; // HTTP/1.0 默认
    }

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

bool RequestParser::clientWantsKeepAlive() const
{
    return _client_wants_keep_alive;
}

} // namespace http