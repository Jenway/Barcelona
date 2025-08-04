// in lib/http/src/RequestRouter.cc
#include "RequestRouter.hpp"
#include "HttpStatus.hpp"
#include "Message.hpp"
#include "ResponseFactory.hpp"

namespace http {

void RequestRouter::addRoute(const std::string& path_prefix, std::unique_ptr<IRequestHandler> handler)
{
    _prefix_routes[path_prefix] = std::move(handler);
}

void RequestRouter::addRoute(Method method, const std::string& exact_path, ConcreteHandler handler)
{
    _exact_routes[{ method, exact_path }] = std::move(handler);
}

auto RequestRouter::handleError(http::StatusCode code) -> Response
{
    return http::responses::createStockResponse(code);
}

auto RequestRouter::handleRequest(const Request& request) -> std::expected<Response, std::error_code>
{
    // 1. 优先匹配精确路由
    if (auto it = _exact_routes.find({ request.method, request.uri }); it != _exact_routes.end()) {
        const auto& handler = it->second;
        return handler(request);
    }

    // 2. 其次匹配前缀路由（最长前缀匹配）
    std::string best_match_prefix;
    IRequestHandler* best_handler = nullptr;

    for (const auto& [prefix, handler] : _prefix_routes) {
        if (request.uri.starts_with(prefix) && prefix.length() > best_match_prefix.length()) {
            best_match_prefix = prefix;
            best_handler = handler.get();
        }
    }

    if (best_handler != nullptr) {
        return best_handler->handleRequest(request);
    }

    // 3. 如果没有任何路由匹配，返回 404
    return http::responses::createStockResponse<http::StatusCode::NotFound>();
}

} // namespace http