// in lib/http/include/http/handlers/StaticFileHandler.hpp
#pragma once

#include "http/common/Message.hpp"
#include <expected>
#include <filesystem>
#include <system_error>

namespace http {

// 一个纯粹的业务逻辑服务类
class StaticFileHandler {
public:
    explicit StaticFileHandler(std::filesystem::path root_path, std::string location_prefix);

    // 统一的请求处理入口
    auto handleRequest(const Request& request) -> Response;

private:
    // 将不同方法的处理逻辑拆分到私有方法中，保持清晰
    auto handleGetOrHead(const Request& request) -> Response;
    auto handleDelete(const Request& request) -> Response;

    const std::filesystem::path _root_path;
    std::string _location_prefix;

    std::string index_file_;
};

} // namespace http