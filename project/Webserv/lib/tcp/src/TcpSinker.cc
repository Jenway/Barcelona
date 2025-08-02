#include "TcpSinker.hpp"
#include "ErrorCode.hpp"
#include "Status.hpp"
#include "net_utils.hpp" // 为了可移植的 sendfile
#include <expected>
#include <unistd.h> // For write()

auto TcpSinker::write(const char* data, size_t len)
    -> std::expected<core::WriteResult, std::error_code>
{
    if (fd_ == -1) {
        return std::unexpected(ErrorCode::Fd_NotSet);
    }
    if (len == 0) {
        return core::WriteResult { .status = core::WriteResult::Status::Finished, .bytes_sent = 0 };
    }

    ssize_t bytes_sent = ::write(fd_, data, len);

    if (bytes_sent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 内核缓冲区已满，需要等待下一次 onWritable 事件
            return core::WriteResult { .status = core::WriteResult::Status::Continue, .bytes_sent = 0 };
        }
        // 真正的 I/O 错误
        return std::unexpected(make_system_error());
    }

    if (static_cast<size_t>(bytes_sent) < len) {
        return core::WriteResult { .status = core::WriteResult::Status::Continue, .bytes_sent = static_cast<size_t>(bytes_sent) };
    }

    // 所有数据都成功写入
    return core::WriteResult { .status = core::WriteResult::Status::Finished, .bytes_sent = static_cast<size_t>(bytes_sent) };
}

auto TcpSinker::sendfile(int in_fd, off_t& offset, size_t count)
    -> std::expected<core::WriteResult, std::error_code>
{
    if (fd_ == -1) {
        return std::unexpected(ErrorCode::Fd_NotSet);
    }
    if (count == 0) {
        return core::WriteResult { .status = core::WriteResult::Status::Finished, .bytes_sent = 0 };
    }

    // 将 offset 的地址传给底层的 sendfile
    ssize_t bytes_sent = ::sendfile(fd_, in_fd, &offset, count);

    auto result = net_utils::sendfile(fd_, in_fd, nullptr, count);

    if (!result) {
        const auto& err = result.error();
        // 检查是否是 EAGAIN
        if (err == std::errc::resource_unavailable_try_again) {
            return core::WriteResult { .status = core::WriteResult::Status::Continue, .bytes_sent = 0 };
        }
        return std::unexpected(err);
    }

    if (static_cast<size_t>(bytes_sent) < count) {
        return core::WriteResult { .status = core::WriteResult::Status::Continue, .bytes_sent = static_cast<size_t>(bytes_sent) };
    }

    return core::WriteResult { .status = core::WriteResult::Status::Finished, .bytes_sent = static_cast<size_t>(bytes_sent) };
}