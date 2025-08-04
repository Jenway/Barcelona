// in lib/http/include/ResponseFactory.hpp
#pragma once

#include "HttpStatus.hpp"
#include "Message.hpp"

namespace http::responses {

/**
 * @brief (模板) 创建一个标准的、无特殊主体的响应。
 *
 * @tparam Code 一个 http::StatusCode 枚举值
 * @return 一个配置好的 http::Response 对象。
 */
template <StatusCode Code>
auto createStockResponse() -> Response
{
    Response response;

    constexpr auto code_int = static_cast<std::underlying_type_t<StatusCode>>(Code);
    constexpr auto reason_sv = magic_enum::enum_name(Code);

    response.status_code = code_int;
    response.reason_phrase = reasonPhraseFromEnum(reason_sv);

    if constexpr (code_int >= 400) {
        response.headers["Connection"] = "close";
        std::string body_str = std::to_string(code_int) + " " + response.reason_phrase;
        response.headers["Content-Type"] = "text/plain; charset=utf-8";
        response.headers["Content-Length"] = std::to_string(body_str.size());
        response.body = std::vector<char>(body_str.begin(), body_str.end());
    }

    return response;
}

/**
 * @brief (运行时) 根据一个 StatusCode 变量创建一个标准的响应。
 *
 * @param code 一个 http::StatusCode 枚举变量。
 * @return 一个配置好的 http::Response 对象。
 */
auto createStockResponse(StatusCode code) -> Response;

auto createText(std::string body, std::string_view content_type = "text/plain; charset=utf-8") -> Response;
auto createJson(std::string json_body) -> Response;

} // namespace http::responses