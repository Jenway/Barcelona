// in lib/http/include/PrefixRouter.hpp
#pragma once
#include "http/interfaces/IRequestHandler.hpp"
#include <map>
#include <memory>
#include <string>

namespace http {
// PrefixRouter 实现了 IRequestHandler 接口
// 它的职责是：根据“最长前缀”来匹配 URI，并将请求分发给对应的子处理器
class PrefixRouter : public http::IRequestHandler {
public:
    // 注册一个子处理器
    void addHandler(const std::string& prefix, std::unique_ptr<http::IRequestHandler> handler);

    // 实现 IRequestHandler 的核心方法
    auto handleRequest(const http::Request& request)
        -> std::expected<http::Response, std::error_code> override;

private:
    std::map<std::string, std::unique_ptr<http::IRequestHandler>> _handlers;
};
}
