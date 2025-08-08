// in lib/http/include/http/handlers/UploadHandler.hpp
#pragma once
#include "http/interfaces/IRequestHandler.hpp"
#include <filesystem>

namespace http {

class UploadHandler : public IRequestHandler {
public:
    // 构造时必须提供一个安全的上传根目录
    explicit UploadHandler(std::filesystem::path upload_root);

    auto handleRequest(const Request& request)
        -> std::expected<Response, std::error_code> override;

private:
    const std::filesystem::path _upload_root;
};

} // namespace http
