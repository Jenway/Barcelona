// in lib/http/include/ResponseFactory.hpp
#pragma once

#include "FileUtils.hpp"
#include "http/common/HttpStatus.hpp"
#include "http/common/Message.hpp"

namespace http::responses {

/**
 * @brief (模板) 创建一个标准的、无特殊主体的响应。
 *
 * @tparam Code 一个 http::StatusCode 枚举值
 * @return 一个配置好的 http::Response 对象。
 */
template <StatusCode Code>
auto

createStockResponse() -> Response
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

/**
 * @brief 根据文件信息和文件描述符，创建一个完整的 200 OK 响应。
 *
 * 该函数会自动设置状态码, Content-Type, Content-Length, 和 FileBody。
 *
 * @param file_info 包含了文件大小和路径的 FileInfo 结构体。
 * @param fd 已打开的文件的有效文件描述符。
 * @return 一个配置好的 http::Response 对象。
 */
auto createFromFile(const utils::FileInfo& file_info, int fd) -> Response;

/**
 * @brief 为 HEAD 请求，根据文件信息创建一个只有头部的 200 OK 响应。
 *
 * @param file_info 包含了文件大小和路径的 FileInfo 结构体。
 * @return 一个包含正确头部但没有主体的 http::Response 对象。
 */
auto createHeaderOnly(const utils::FileInfo& file_info) -> Response;
} // namespace http::responses