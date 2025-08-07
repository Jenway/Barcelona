// in lib/utils/src/FileUtils.cc
#include "FileUtils.hpp"
#include "Error.hpp"
#include "FileDescriptor.hpp"
#include <expected>
#include <fcntl.h>
#include <filesystem>
#include <map>
#include <string>
#include <system_error>

namespace utils {

// Helper: convert a single hex digit to integer, or -1 on invalid
static int hexDigit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

// URL-decode a percent-encoded string in-place (returns false on invalid encoding)
bool url_decode(std::string& s)
{
    std::string result;
    result.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int hi = hexDigit(s[i + 1]);
            int lo = hexDigit(s[i + 2]);
            if (hi < 0 || lo < 0)
                return false;
            char decoded = static_cast<char>((hi << 4) | lo);
            result.push_back(decoded);
            i += 2;
        } else if (s[i] == '+') {
            result.push_back(' ');
        } else {
            result.push_back(s[i]);
        }
    }
    s.swap(result);
    return true;
}

// Normalize and sanitize a URI path (removes query, fragment, decodes percent-encoding)
auto normalizeUriPath(std::string_view raw_uri)
    -> std::expected<std::string, std::error_code>
{
    // Must start with '/'
    if (raw_uri.empty() || raw_uri.front() != '/')
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));

    // Strip query and fragment
    auto end_pos = raw_uri.find_first_of("?#");
    std::string path = std::string(raw_uri.substr(0, end_pos));

    // URL-decode
    if (!url_decode(path))
        return std::unexpected(std::make_error_code(std::errc::illegal_byte_sequence));

    // Collapse redundant slashes
    size_t pos;
    while ((pos = path.find("//")) != std::string::npos)
        path.erase(pos, 1);

    // Use filesystem normalization
    std::filesystem::path fs_path = std::filesystem::path(path).lexically_normal();

    // Prevent escaping above root
    auto s = fs_path.generic_string();
    if (s.empty() || s.rfind("..", 0) == 0)
        return std::unexpected(std::make_error_code(std::errc::permission_denied));

    return s;
}

// Resolve a safe filesystem path under doc_root
// index_file should be e.g. "index.html"
auto resolveSafePath(
    const std::filesystem::path& doc_root,
    std::string_view raw_uri,
    std::string_view index_file)
    -> std::expected<std::filesystem::path, std::error_code>
{

    std::string rel = std::string(raw_uri);
    if (rel == "/" || rel.empty())
        rel = index_file;
    else if (rel.front() == '/')
        rel.erase(0, 1);

    if (rel.find("..") != std::string::npos) {
        return std::unexpected(std::make_error_code(std::errc::permission_denied));
    }
    // Combine and normalize again
    std::filesystem::path full = (doc_root / rel).lexically_normal();

    // Ensure full stays within doc_root
    auto [it_root, it_full] = std::mismatch(
        doc_root.begin(), doc_root.end(), full.begin());
    if (it_root != doc_root.end())
        return std::unexpected(std::make_error_code(std::errc::permission_denied));

    return full;
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