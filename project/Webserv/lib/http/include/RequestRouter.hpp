// in lib/http/include/RequestRouter.hpp
#pragma once

#include "IRequestHandler.hpp"
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace http {

// 为了灵活性，我们定义一个通用的处理函数类型
using ConcreteHandler = std::function<std::expected<Response, std::error_code>(const Request&)>;

class RequestRouter : public IRequestHandler {
public:
    RequestRouter() = default;

    // --- 注册路由的接口 ---

    // 注册一个能处理特定前缀路径的处理器
    void addRoute(const std::string& path_prefix, std::unique_ptr<IRequestHandler> handler);

    // 或者，直接注册一个能处理特定方法和精确路径的函数
    void addRoute(Method method, const std::string& exact_path, ConcreteHandler handler);

    // --- IRequestHandler 接口实现 ---

    // 核心的路由分发逻辑
    auto handleRequest(const Request& request)
        -> std::expected<Response, std::error_code> override;

    // 通用的 400 错误
    auto handleError(http::StatusCode code) -> Response override;

private:
    // 简单的路由表结构
    // Key 是路径前缀，Value 是对应的处理器
    std::map<std::string, std::unique_ptr<IRequestHandler>> _prefix_routes;

    std::map<std::pair<Method, std::string>, ConcreteHandler> _exact_routes;
};

} // namespace http