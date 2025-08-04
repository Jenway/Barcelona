// in lib/http/include/ExactRouter.hpp
#pragma once
#include "http/interfaces/IRequestHandler.hpp"

namespace http {
using ConcreteHandler = std::function<std::expected<http::Response, std::error_code>(const http::Request&)>;

// ExactRouter 也实现了 IRequestHandler 接口
// 它的职责是：根据“方法+精确路径”来匹配请求
class ExactRouter : public http::IRequestHandler {
public:
    void addHandler(http::Method method, const std::string& path, ConcreteHandler handler);

    auto handleRequest(const http::Request& request)
        -> std::expected<http::Response, std::error_code> override;

private:
    std::map<std::pair<http::Method, std::string>, ConcreteHandler> _handlers;
};
}
