
// in lib/http/src/StaticFileHandler.cc
#include "http/handlers/StaticFileHandler.hpp"
#include "FileUtils.hpp"
#include "http/core/HttpStatus.hpp"
#include "http/utils/ResponseFactory.hpp"
#include <expected>
#include <fcntl.h>
#include <system_error>
#include <unistd.h>

namespace http {

StaticFileHandler::StaticFileHandler(std::filesystem::path root_path)
    : _root_path(std::move(root_path))
{
}

auto StaticFileHandler::handleRequest(const Request& request) -> std::expected<Response, std::error_code>
{
    if (request.method != Method::GET) {
        return responses::createStockResponse<StatusCode::MethodNotAllowed>();
    }

    // 1. 解析路径
    auto path_or_error = utils::resolveSafePath(_root_path, request.uri);
    if (!path_or_error) {
        return responses::createStockResponse<StatusCode::BadRequest>();
    }

    // 2. 获取文件信息
    auto info_or_error = utils::getFileInfo(*path_or_error);
    if (!info_or_error) {
        const auto& ec = info_or_error.error();
        if (ec == std::errc::no_such_file_or_directory) {
            return responses::createStockResponse<StatusCode::NotFound>();
        }
        if (ec == std::errc::permission_denied) {
            return responses::createStockResponse<StatusCode::Forbidden>();
        }
        return responses::createStockResponse<StatusCode::InternalServerError>();
    }

    // 3. 打开文件
    auto fd_or_error = utils::openFileForReading(info_or_error->full_path);
    if (!fd_or_error) {
        const auto& ec = fd_or_error.error();
        if (ec == std::errc::permission_denied) {
            return responses::createStockResponse<StatusCode::Forbidden>();
        }
        return responses::createStockResponse<StatusCode::InternalServerError>();
    }

    // 4. 创建响应
    return responses::createFromFile(*info_or_error, fd_or_error->release());
}

} // namespace http
