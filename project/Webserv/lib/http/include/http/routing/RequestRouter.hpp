// in lib/http/include/RequestRouter.hpp
#pragma once

#include "http/interfaces/IRequestHandler.hpp"
#include <memory>

namespace http {

class RequestRouter : public IRequestDispatcher {
public:
    RequestRouter() = default;

    void use(std::unique_ptr<http::IRequestHandler> handler);

    auto handleError(http::StatusCode code) -> Response override;
    auto handleRequest(const http::Request& request)
        -> std::expected<http::Response, std::error_code> override;

private:
    std::vector<std::unique_ptr<http::IRequestHandler>> _pipeline;
};

} // namespace http
