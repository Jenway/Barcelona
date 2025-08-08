// in lib/http/src/handlers/StaticFileHandler.cc
#include <utility>

#include "FileUtils.hpp"
#include "http/common/HttpStatus.hpp"
#include "http/handlers/StaticFileHandler.hpp"
#include "http/utils/ResponseFactory.hpp"
#include "logger.hpp"

namespace http {

StaticFileHandler::StaticFileHandler(std::filesystem::path root_path, std::string location_prefix)
    : _root_path(std::move(root_path))
    , _location_prefix(std::move(location_prefix))
{
}

// 统一的公共入口，根据方法进行分发
auto StaticFileHandler::handleRequest(const Request& request) -> Response
{
    switch (request.method) {
    case Method::GET:
    case Method::HEAD:
        return handleGetOrHead(request);
    case Method::DELETE:
        return handleDelete(request);
    default:
        // 对于其他所有不支持的方法（POST, PUT, etc.）
        return responses::createStockResponse<StatusCode::MethodNotAllowed>();
    }
}

// 私有的 GET/HEAD 处理器
auto StaticFileHandler::handleGetOrHead(const Request& request) -> Response
{
    std::string sub_uri = request.uri;
    if (sub_uri.starts_with(_location_prefix)) {
        sub_uri.erase(0, _location_prefix.length());
        if (!sub_uri.starts_with('/')) {
            sub_uri = "/" + sub_uri;
        }
    }
    auto path_or_error = utils::resolveSafePath(_root_path, sub_uri);
    if (!path_or_error) {
        return responses::createStockResponse<StatusCode::BadRequest>();
    }

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

    // 如果是 HEAD 请求，调用新的工厂函数并直接返回
    if (request.method == Method::HEAD) {
        return responses::createHeaderOnly(*info_or_error);
    }

    // --- 以下是 GET 请求的专属逻辑 ---
    auto fd_or_error = utils::openFileForReading(info_or_error->full_path);
    if (!fd_or_error) {
        if (fd_or_error.error() == std::errc::permission_denied) {
            return responses::createStockResponse<StatusCode::Forbidden>();
        }
        return responses::createStockResponse<StatusCode::InternalServerError>();
    }

    return responses::createFromFile(*info_or_error, fd_or_error->release());
}

// 私有的 DELETE 处理器
auto StaticFileHandler::handleDelete(const Request& request) -> Response
{
    LOG_DEBUG("Deleting file info for {}", request.uri);

    std::string sub_uri = request.uri;
    if (sub_uri.starts_with(_location_prefix)) {
        sub_uri.erase(0, _location_prefix.length());
        if (!sub_uri.starts_with('/')) {
            sub_uri = "/" + sub_uri;
        }
    }
    auto path_or_error = utils::resolveSafePath(_root_path, sub_uri);
    if (!path_or_error) {
        LOG_ERROR("Error resolving {} : {}", request.uri, path_or_error.error());
        return responses::createStockResponse<StatusCode::BadRequest>();
    }

    // 检查文件是否存在且为常规文件，getFileInfo 可以复用
    auto info_or_error = utils::getFileInfo(*path_or_error);
    if (!info_or_error) {
        LOG_ERROR("Error getting file info for {} : {}", request.uri, info_or_error.error());

        // 如果文件不存在，返回 404
        if (info_or_error.error() == std::errc::no_such_file_or_directory) {
            return responses::createStockResponse<StatusCode::NotFound>();
        }
        // 如果是目录或权限问题，返回 403
        return responses::createStockResponse<StatusCode::Forbidden>();
    }

    std::error_code ec;
    if (!std::filesystem::remove(info_or_error->full_path, ec)) {
        LOG_ERROR("Error removing file {} : {}", request.uri, info_or_error.error());

        if (ec == std::errc::permission_denied) {
            return responses::createStockResponse<StatusCode::Forbidden>();
        }
        return responses::createStockResponse<StatusCode::InternalServerError>();
    }

    // 删除成功，返回 204 No Content
    return responses::createStockResponse<StatusCode::NoContent>();
}

} // namespace http