#pragma once

#include "http/common/Message.hpp"
#include "http/interfaces/IRequestParser.hpp"
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace http {

class RequestParser : public IRequestParser {
public:
    explicit RequestParser(
        size_t max_body_size = std::numeric_limits<size_t>::max());
    void reset() override;
    auto parse(std::string_view data) -> std::expected<IRequestParser::State, StatusCode> override;
    [[nodiscard]] auto getRequest() const -> const Request& override;
    [[nodiscard]] bool clientWantsKeepAlive() const override;

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
    auto parseRequestLine() -> std::expected<ParseResult, StatusCode>;
    auto parseHeaders() -> std::expected<ParseResult, StatusCode>;
    auto parseBody() -> std::expected<ParseResult, StatusCode>;

    Step _step;
    size_t _max_body_size;
    bool _client_wants_keep_alive = false;
    std::string _buffer;
    Request _request;
};

} // namespace http
