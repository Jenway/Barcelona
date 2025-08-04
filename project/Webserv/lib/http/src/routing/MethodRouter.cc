// in lib/http/src/MethodRouter.cc
#include "http/routing/MethodRouter.hpp"
#include "http/utils/ResponseFactory.hpp"

void MethodRouter::addHandler(http::Method method, std::unique_ptr<http::IRequestHandler> handler)
{
    _handlers[method] = std::move(handler);
}

auto MethodRouter::handleRequest(const http::Request& request) -> std::expected<http::Response, std::error_code>
{
    // 在 map 中查找当前请求的方法
    if (auto it = _handlers.find(request.method); it != _handlers.end()) {
        // 如果找到了，就委托给对应的子处理器
        return it->second->handleRequest(request);
    }

    // 如果这个 MethodRouter 不支持该方法，返回 405 Method Not Allowed
    return http::responses::createStockResponse<http::StatusCode::MethodNotAllowed>();
}