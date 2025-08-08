#pragma once
#include "http/interfaces/IRequestHandler.hpp"

namespace http {

class RedirectHandler : public IRequestHandler {
public:
    // 构造时需要知道重定向的状态码 (通常是 301 或 302) 和目标 URL
    RedirectHandler(int status_code, std::string target_url);

    auto handleRequest(const Request& request)
        -> std::expected<Response, std::error_code> override;

private:
    int _status_code;
    std::string _target_url;
};
}