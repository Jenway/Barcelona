// in lib/http/src/handlers/RedirectHandler.cc
#include "http/handlers/RedirectHandler.hpp"
#include "http/core/HttpStatus.hpp"
#include "http/utils/ResponseFactory.hpp"
#include "logger.hpp"

namespace http {

RedirectHandler::RedirectHandler(int status_code, std::string target_url)
    : _status_code(status_code)
    , _target_url(std::move(target_url))
{
}

auto RedirectHandler::handleRequest(const Request& /*request*/) -> std::expected<Response, std::error_code>
{
    // 将 int 转换为我们类型安全的 StatusCode 枚举
    auto status_code_opt = magic_enum::enum_cast<StatusCode>(_status_code);

    Response response;

    if (status_code_opt && (*status_code_opt >= StatusCode::MultipleChoices && *status_code_opt < StatusCode::BadRequest)) {
        // 如果是合法的重定向代码 (3xx)
        response = responses::createStockResponse(*status_code_opt);
    } else {
        // 如果提供了一个无效的重定向代码，则返回一个服务器内部错误作为 fallback
        LOG_WARN("Invalid redirect code {} used. Falling back to 500.", _status_code);
        response = responses::createStockResponse<StatusCode::InternalServerError>();
        return response;
    }

    // **重定向的核心：设置 Location 头部**
    response.headers["Location"] = _target_url;

    // 根据 RFC 规范，重定向响应通常会包含一个简短的 HTML 主体，
    // 以便在不支持自动重定向的旧客户端上显示一个链接。
    std::string body_str = "<!DOCTYPE html><html><head><title>Redirect</title></head><body>"
                           "<p>This page has moved to <a href=\""
        + _target_url + "\">" + _target_url + "</a>.</p>"
                                              "</body></html>";

    response.headers["Content-Type"] = "text/html; charset=utf-8";
    response.headers["Content-Length"] = std::to_string(body_str.size());
    response.body = std::vector<char>(body_str.begin(), body_str.end());

    return response;
}

} // namespace http