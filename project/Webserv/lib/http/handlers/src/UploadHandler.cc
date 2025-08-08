// in lib/http/src/handlers/UploadHandler.cc
#include "http/handlers/UploadHandler.hpp"
#include "http/common/HttpStatus.hpp"
#include "http/utils/ResponseFactory.hpp"
#include "logger.hpp"
#include <fstream>

namespace http {

UploadHandler::UploadHandler(std::filesystem::path upload_root)
    : _upload_root(std::move(upload_root))
{
    // 在启动时确保上传目录存在，如果不存在则创建
    std::error_code ec;
    std::filesystem::create_directories(_upload_root, ec);
    if (ec) {
        // 这是一个致命的配置错误，我们应该记录并可能中止
        LOG_ERROR("Failed to create upload directory '{}': {}", _upload_root.string(), ec.message());
        throw std::runtime_error("Upload directory could not be created.");
    }
}

auto UploadHandler::handleRequest(const Request& request) -> std::expected<Response, std::error_code>
{
    // 1. 只允许 POST 和 PUT
    if (request.method != Method::POST && request.method != Method::PUT) {
        return responses::createStockResponse<StatusCode::MethodNotAllowed>();
    }

    // 2. 从 URI 中安全地提取文件名
    //    std::filesystem::path(...).filename() 是一个强大的工具，
    //    它会自动剥离掉任何目录信息，如 /upload/../foo.txt -> foo.txt
    auto filename = std::filesystem::path(request.uri).filename();

    // 安全检查：如果文件名是空的或包含 ".."，则拒绝
    if (filename.empty() || filename.string().find("..") != std::string::npos) {
        return responses::createStockResponse<StatusCode::BadRequest>();
    }

    // 3. 构建完整的目标路径
    auto destination_path = _upload_root / filename;

    // 4. 将请求体写入文件
    //    以二进制模式打开，以处理任何类型的文件
    std::ofstream outfile(destination_path, std::ios::binary);
    if (!outfile) {
        LOG_ERROR("Failed to open file for writing: {}", destination_path.string());
        return responses::createStockResponse<StatusCode::InternalServerError>();
    }

    outfile.write(request.body.data(), request.body.size());
    if (outfile.fail()) {
        LOG_ERROR("Failed to write to file: {}", destination_path.string());
        // 写入失败后，最好尝试删除已创建的不完整文件
        std::filesystem::remove(destination_path);
        return responses::createStockResponse<StatusCode::InternalServerError>();
    }

    LOG_INFO("Successfully uploaded file to {}", destination_path.string());

    // 5. 返回成功的响应
    //    根据测试脚本的期望，返回一个 JSON
    std::string json_body = R"({"message": "File uploaded successfully"})";

    // 我们可以创建一个新的 ResponseFactory 函数来专门处理 201
    Response response = responses::createJson(std::move(json_body));
    response.status_code = 201; // <--- 设置状态码为 201 Created
    response.reason_phrase = "Created";

    return response;
}

} // namespace http