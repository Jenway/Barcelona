#pragma once

#include "ErrorCode.hpp"
#include "IRequestParser.hpp"
#include "Message.hpp"
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace http {

class RequestParser : IRequestParser {
public:
    RequestParser();

    void reset() override;
    auto parse(std::string_view data) -> std::expected<IRequestParser::State, std::error_code> override;
    [[nodiscard]] auto getRequest() const -> const Request& override;

private:
    enum class Step : uint8_t {
        RequestLine,
        Headers,
        Body,
        Completed
    };
    enum class ParseResult : uint8_t {
        Success,
        Incomplete
    };
    auto parseRequestLine() -> std::expected<ParseResult, ErrorCode>;
    auto parseHeaders() -> std::expected<ParseResult, ErrorCode>;
    auto parseBody() -> std::expected<ParseResult, ErrorCode>;

    Step _step;
    std::string _buffer;
    Request _request;
};

} // namespace http
