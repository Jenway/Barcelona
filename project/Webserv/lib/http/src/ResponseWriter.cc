#include "ResponseWriter.hpp"
#include "Message.hpp"
#include "Serializer.hpp"
#include <cstdio>
#include <utility>

namespace http {

void ResponseWriter::bind_to(http::Response response)
{
    _response = std::move(response);
    _serialized_headers = http::serialize_headers(_response);
}

bool ResponseWriter::isKeepAlive() const
{
    if (_response.headers.contains("Connection")) {
        return _response.headers.at("Connection") == "keep-alive";
    }
    return false; // 默认不保持连接
}

auto ResponseWriter::writeTo(ISinker& sinker) -> std::expected<core::WriteResult, std::error_code>
{
    // 循环以允许在一次调用中从发送头无缝过渡到发送体
    while (true) {
        if (_state == State::SendingHeaders) {
            auto res = sendHeaders(sinker);
            // 如果头没有写完（被阻塞）或出错，则直接返回结果
            if (!res || res->status != core::WriteResult::Status::Finished) {
                return res;
            }

            // 头已发送完毕，立即转换状态，并让循环继续，以便尝试发送Body
            _state = State::SendingBody;
        }

        if (_state == State::SendingBody) {
            auto res = sendBody(sinker);
            // 无论 Body 是否发送完毕，都返回它的结果。
            // 这将是此次 writeTo 调用的最终结果。
            if (res && res->status == core::WriteResult::Status::Finished) {
                _state = State::Finished;
            }
            return res;
        }

        if (_state == State::Finished) {
            return core::WriteResult { .status = core::WriteResult::Status::Finished, .bytes_sent = 0 };
        }
    }
}

auto ResponseWriter::sendHeaders(ISinker& sinker) -> std::expected<core::WriteResult, std::error_code>
{
    const char* data = _serialized_headers.data() + _header_bytes_sent;
    size_t len = _serialized_headers.size() - _header_bytes_sent;
    auto res = sinker.write(data, len);
    if (res) {
        _header_bytes_sent += res->bytes_sent;
    }
    return res;
}

auto ResponseWriter::sendBody(ISinker& sinker) -> std::expected<core::WriteResult, std::error_code>
{
    return std::visit(
        [&](auto&& body) -> std::expected<core::WriteResult, std::error_code> {
            using T = std::decay_t<decltype(body)>;

            if constexpr (std::is_same_v<T, std::vector<char>>) {
                // Handle in-memory body
                const char* data = body.data() + _body_bytes_sent;
                size_t len = body.size() - _body_bytes_sent;
                auto res = sinker.write(data, len);
                if (res) {
                    _body_bytes_sent += res->bytes_sent;
                }
                return res;
            } else if constexpr (std::is_same_v<T, FileBody>) {
                return sinker.sendfile(body.fd, body.offset, body.size);
            }
        },
        _response.body);
}

} // namespace http