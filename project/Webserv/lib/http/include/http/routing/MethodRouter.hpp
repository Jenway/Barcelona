// in lib/http/include/MethodRouter.hpp
#pragma once
#include "http/interfaces/IRequestHandler.hpp"
#include <map>
#include <memory>

class MethodRouter : public http::IRequestHandler {
public:
    // 为特定的方法注册一个处理器
    void addHandler(http::Method method, std::unique_ptr<http::IRequestHandler> handler);

    auto handleRequest(const http::Request& request)
        -> std::expected<http::Response, std::error_code> override;

private:
    std::map<http::Method, std::unique_ptr<http::IRequestHandler>> _handlers;
};