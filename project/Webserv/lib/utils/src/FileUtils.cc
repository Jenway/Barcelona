// in lib/utils/src/FileUtils.cc
#include "FileUtils.hpp"
#include "Error.hpp"
#include "FileDescriptor.hpp"
#include <fcntl.h>
#include <map>

namespace utils {

auto resolveSafePath(
    const std::filesystem::path& doc_root,
    const std::string& uri,
    const std::string& index_file)
    -> std::expected<std::filesystem::path, std::error_code>
{
    // 1. 基础验证
    if (uri.empty() || uri[0] != '/' || uri.find("..") != std::string::npos) {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    // 2. 拼接相对路径
    std::filesystem::path relative_path;
    if (uri == "/") {
        relative_path = index_file;
    } else {
        relative_path = uri.substr(1); // 移除开头的 '/'
    }

    // 3. 组合并规范化路径
    // lexically_normal 会处理 "." 和 ".." 等，但它不访问文件系统
    auto final_path = (doc_root / relative_path).lexically_normal();

    // 4. 最关键的安全检查：防止路径遍历攻击
    // 我们通过比较两个路径的公共前缀长度来确保 final_path 仍然在 doc_root 内部。
    // std::distance(mismatch(...)) 计算了公共部分的长度。
    auto const common_len = std::distance(
        doc_root.begin(),
        std::mismatch(doc_root.begin(), doc_root.end(), final_path.begin()).first);

    if (static_cast<size_t>(common_len) < std::distance(doc_root.begin(), doc_root.end())) {
        // 如果公共部分的长度小于 doc_root 的长度，说明路径已经逃逸出去了。
        return std::unexpected(std::make_error_code(std::errc::permission_denied));
    }

    return final_path;
}

auto getFileInfo(const std::filesystem::path& path) -> std::expected<FileInfo, std::error_code>
{
    std::error_code ec;
    const auto status = std::filesystem::status(path, ec);

    if (ec) {
        return std::unexpected(ec); // status() 调用本身失败
    }
    if (!std::filesystem::exists(status)) {
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    }
    if (!std::filesystem::is_regular_file(status)) {
        // 对于目录、符号链接等，我们视为一种“权限不足”的错误
        return std::unexpected(std::make_error_code(std::errc::permission_denied));
    }

    const auto file_size = std::filesystem::file_size(path, ec);
    if (ec) {
        return std::unexpected(ec); // file_size() 调用失败
    }

    return FileInfo { .full_path = path, .size = file_size };
}

auto getMimeType(const std::filesystem::path& path) -> std::string
{
    static const std::map<std::string, std::string> mime_types = {
        { ".html", "text/html; charset=utf-8" },
        { ".htm", "text/html; charset=utf-8" },
        { ".css", "text/css" },
        { ".js", "application/javascript" },
        { ".json", "application/json" },
        { ".txt", "text/plain" },
        { ".jpg", "image/jpeg" },
        { ".jpeg", "image/jpeg" },
        { ".png", "image/png" },
        { ".gif", "image/gif" },
        { ".svg", "image/svg+xml" },
        { ".xml", "application/xml" },
        { ".pdf", "application/pdf" }
    };

    const std::string extension = path.extension().string();
    if (auto it = mime_types.find(extension); it != mime_types.end()) {
        return it->second;
    }

    return "application/octet-stream"; // 默认 MIME 类型
}

auto openFileForReading(const std::filesystem::path& path) -> std::expected<FileDescriptor, std::error_code>
{
    const int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        return error::to_unexpected_code();
    }
    return FileDescriptor(fd);
}

} // namespace utils