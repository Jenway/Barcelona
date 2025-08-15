#pragma once

#include "http/common/HttpStatus.hpp"
#include "http/common/Message.hpp"
#include <expected>
#include <system_error>

namespace http {

class IRequestHandler {
public:
    virtual ~IRequestHandler() = default;

    virtual auto handleRequest(const Request& request)
        -> std::expected<Response, std::error_code>
        = 0;
};

class IRequestDispatcher {
public:
    virtual ~IRequestDispatcher() = default;

    virtual auto handleRequest(const Request& request)
        -> std::expected<Response, std::error_code>
        = 0;
    virtual auto handleError(StatusCode code) -> Response = 0;
};

} // namespace http