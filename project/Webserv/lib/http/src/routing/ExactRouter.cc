// in lib/http/src/ExactRouter.cc
#include "http/routing/ExactRouter.hpp"
#include "http/utils/ResponseFactory.hpp"
#include <utility>

namespace http {

void ExactRouter::addHandler(http::Method method, const std::string& path, ConcreteHandler handler)
{
    _handlers[std::make_pair(method, path)] = std::move(handler);
}

auto ExactRouter::handleRequest(const http::Request& request) -> std::expected<http::Response, std::error_code>
{
    // 这段逻辑也是从您旧的 RequestRouter 中“抠”出来的
    if (auto it = _handlers.find({ request.method, request.uri }); it != _handlers.end()) {
        const auto& handler = it->second;
        return handler(request);
    }

    // 如果这个 ExactRouter 内部没有匹配，它也返回 404
    return http::responses::createStockResponse<http::StatusCode::NotFound>();
}
}
