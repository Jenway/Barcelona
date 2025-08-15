// in lib/http/include/http/routing/ExactRouter.hpp
#pragma once
#include "http/interfaces/IRequestHandler.hpp"
#include "http/utils/ResponseFactory.hpp" // 需要包含
#include <utility> // 需要为 std::move 和 std::pair

namespace http {
using ConcreteHandler = std::function<std::expected<http::Response, std::error_code>(const http::Request&)>;

class ExactRouter : public IRequestHandler {
public:
    void addHandler(Method method, const std::string& path, ConcreteHandler handler)
    {
        _handlers[std::make_pair(method, path)] = std::move(handler);
    }

    auto handleRequest(const Request& request) -> std::expected<Response, std::error_code> override
    {
        if (auto it = _handlers.find({ request.method, request.uri }); it != _handlers.end()) {
            return it->second(request);
        }
        return responses::createStockResponse<StatusCode::NotFound>();
    }

private:
    std::map<std::pair<Method, std::string>, ConcreteHandler> _handlers;
};
} // namespace http