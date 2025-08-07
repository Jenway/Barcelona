// in lib/http/src/PrefixRouter.cc
#include "http/routing/PrefixRouter.hpp"
#include "http/utils/ResponseFactory.hpp" // 用于 404

namespace http {
void PrefixRouter::addHandler(const std::string& prefix, std::unique_ptr<http::IRequestHandler> handler)
{
    _handlers[prefix] = std::move(handler);
}

auto PrefixRouter::handleRequest(const http::Request& request) -> std::expected<http::Response, std::error_code>
{
    std::string best_match_prefix;
    IRequestHandler* best_handler = nullptr;

    for (const auto& [prefix, handler] : _handlers) {
        if (request.uri.starts_with(prefix) && prefix.length() > best_match_prefix.length()) {
            best_match_prefix = prefix;
            best_handler = handler.get();
        }
    }

    if (best_handler != nullptr) {
        return best_handler->handleRequest(request);
    }

    // 如果这个 PrefixRouter 内部没有任何匹配，它就返回 404
    return http::responses::createStockResponse<http::StatusCode::NotFound>();
}
} // namespace http
