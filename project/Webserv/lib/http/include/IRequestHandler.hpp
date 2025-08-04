#pragma once

#include "HttpStatus.hpp"
#include "Message.hpp"
#include <expected>
#include <system_error>

namespace http {

class IRequestHandler {
public:
    virtual ~IRequestHandler() = default;

    virtual auto handleRequest(const Request& request)
        -> std::expected<Response, std::error_code>
        = 0;

    virtual auto handleError(http::StatusCode) -> Response = 0;
};

} // namespace http