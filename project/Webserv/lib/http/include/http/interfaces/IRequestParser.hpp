// in lib/http/include/IRequestParser.hpp
#pragma once

#include "http/core/HttpStatus.hpp"
#include "http/core/Message.hpp"
#include <cstdint>
#include <expected>
#include <string_view>
#include <sys/types.h>

namespace http {

class IRequestParser {
public:
    enum class State : uint8_t {
        Parsing,
        Completed
    };

    virtual ~IRequestParser() = default;
    virtual auto parse(std::string_view data) -> std::expected<IRequestParser::State, StatusCode> = 0;
    [[nodiscard]] virtual auto getRequest() const -> const http::Request& = 0;
    virtual void reset() = 0;
};

} // namespace http
