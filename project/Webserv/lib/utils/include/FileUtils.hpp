// in lib/utils/include/FileUtils.hpp
#pragma once

#include "FileDescriptor.hpp"
#include <expected>
#include <filesystem>
#include <string>
#include <system_error>

namespace utils {

struct FileInfo {
    std::filesystem::path full_path;
    uintmax_t size {};
    // 可以扩展更多信息，如
    // std::filesystem::file_time_type last_modified;
};

/**
 * @brief 对 URI 路径进行规范化和安全检查。
 *
 * 该函数执行以下操作：
 * 1.  使用 std::filesystem::path 解析并处理路径中的 "." 和 ".."。
 * 2.  确保规范化后的路径不会以 ".." 开头，防止路径遍历。
 * 3.  解码 URL 编码的字符 (例如, %20 -> ' ') (可选，但推荐)。
 *
 * @param uri_path 从 HTTP 请求中解析出的原始 URI 路径。
 * @return 成功时返回一个干净、安全的 URI 路径字符串，失败时返回错误码。
 */
auto normalizeUriPath(std::string_view raw_uri)
    -> std::expected<std::string, std::error_code>;

/**
 * @brief 安全地将 URI 解析为文件系统路径，并验证其合法性。
 *
 * 该函数执行以下关键操作：
 * 1.  将 URI 映射到指定的文档根目录下。
 * 2.  处理对根目录 "/" 的请求，自动映射到 index_file (e.g., "index.html")。
 * 3.  对路径进行规范化 (lexically_normal)，解析 "." 和 ".."。
 * 4.  执行关键的安全检查，确保最终路径没有逃逸出文档根目录（防止路径遍历攻击）。
 *
 * @param doc_root 服务器的文档根目录。
 * @param uri 来自 HTTP 请求的 URI。
 * @param index_file 当 URI 为 "/" 时要查找的默认文件。
 * @return 成功时返回包含绝对路径的 std::filesystem::path，失败时返回错误码。
 */
auto resolveSafePath(
    const std::filesystem::path& doc_root,
    std::string_view raw_uri,
    std::string_view index_file = "index.html")
    -> std::expected<std::filesystem::path, std::error_code>;

/**
 * @brief 获取一个文件的详细信息，包括其类型和大小。
 *
 * 该函数封装了对 `std::filesystem::status` 和 `std::filesystem::file_size` 的调用，
 * 并将结果包装在一个结构体中，同时处理了所有可能的文件系统错误。
 *
 * @param path 要查询的文件的路径。
 * @return 成功时返回一个 FileInfo 结构体，失败时返回错误码。
 */
auto getFileInfo(const std::filesystem::path& path)
    -> std::expected<FileInfo, std::error_code>;

/**
 * @brief 根据文件扩展名猜测其 MIME 类型。
 *
 * @param path 文件路径。
 * @return 代表 MIME 类型的字符串，如果无法识别则返回默认值。
 */
auto getMimeType(const std::filesystem::path& path) -> std::string;

/**
 * @brief 以只读方式安全地打开一个文件。
 * @param path 要打开的文件路径。
 * @return 成功时返回一个管理着文件描述符的 FileDescriptor 对象，失败时返回错误码。
 */
auto openFileForReading(const std::filesystem::path& path) -> std::expected<FileDescriptor, std::error_code>;

} // namespace utils