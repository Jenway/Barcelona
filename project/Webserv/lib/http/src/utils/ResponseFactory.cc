// in lib/http/src/ResponseFactory.cc
#include "http/utils/ResponseFactory.hpp"
#include <vector>

namespace http::responses {

auto createStockResponse(StatusCode code) -> Response
{
    Response response;

    // 在运行时使用 magic_enum 获取信息
    response.status_code = magic_enum::enum_integer(code);
    auto reason_sv = magic_enum::enum_name(code);
    response.reason_phrase = reasonPhraseFromEnum(reason_sv);

    if (response.status_code >= 400) {
        response.headers["Connection"] = "close";
        std::string body_str = std::to_string(response.status_code) + " " + response.reason_phrase;
        response.headers["Content-Type"] = "text/plain; charset=utf-8";
        response.headers["Content-Length"] = std::to_string(body_str.size());
        response.body = std::vector<char>(body_str.begin(), body_str.end());
    }

    return response;
}

auto createText(std::string body, std::string_view content_type) -> Response
{
    Response response;
    response.status_code = 200;
    response.reason_phrase = "OK";

    response.headers["Content-Type"] = content_type;
    response.headers["Content-Length"] = std::to_string(body.size());

    response.body = std::vector<char>(
        std::make_move_iterator(body.begin()),
        std::make_move_iterator(body.end()));

    return response;
}

auto createJson(std::string json_body) -> Response
{
    return createText(std::move(json_body), "application/json; charset=utf-8");
}

auto createFromFile(const utils::FileInfo& file_info, int fd) -> Response
{
    Response response = createStockResponse<StatusCode::Ok>();

    // 2. 添加文件特有的头部
    response.headers["Content-Type"] = utils::getMimeType(file_info.full_path);
    response.headers["Content-Length"] = std::to_string(file_info.size);
    response.headers["Connection"] = "close"; // 暂时先关闭

    // 3. 设置 FileBody
    response.body = FileBody { .fd = fd, .size = file_info.size };

    return response;
}

} // namespace http::responses