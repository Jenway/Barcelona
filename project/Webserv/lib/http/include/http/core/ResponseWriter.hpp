#pragma once

#include "ISinker.hpp"
#include "http/core/Message.hpp"
#include "http/interfaces/IResponseWriter.hpp"
#include <expected>
#include <string>

namespace http {

class ResponseWriter : public IResponseWriter {
public:
    [[nodiscard]] bool isKeepAlive() const override;
    auto writeTo(ISinker& sinker) -> std::expected<core::WriteResult, std::error_code> override;
    void bind_to(http::Response response) override;
    void reset() override { };

private:
    auto sendHeaders(ISinker& sinker) -> std::expected<core::WriteResult, std::error_code>;
    auto sendBody(ISinker& sinker) -> std::expected<core::WriteResult, std::error_code>;

    enum class State : uint8_t {
        SendingHeaders,
        SendingBody,
        Finished
    };

    http::Response _response;
    State _state = State::SendingHeaders;
    std::string _serialized_headers;
    size_t _header_bytes_sent = 0;
    size_t _body_bytes_sent = 0;
};

} // namespace http
