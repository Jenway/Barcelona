#include "TcpSource.hpp"
#include "ErrorCode.hpp"
#include <unistd.h> // For read()

auto TcpSource::read(std::vector<char>& buffer)
    -> std::expected<std::pair<core::ReadStatus, size_t>, std::error_code>
{
    if (fd_ == -1) {
        return std::unexpected(ErrorCode::Fd_NotSet);
    }
    ssize_t bytes_read = ::read(fd_, buffer.data(), buffer.size());

    if (bytes_read > 0) {
        return std::make_pair(core::ReadStatus::GotData, bytes_read);
    }

    if (bytes_read == 0) {
        return std::make_pair(core::ReadStatus::Eof, 0);
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // 如果是 EAGAIN，表示当前无数据可读，这是正常情况
        return std::make_pair(core::ReadStatus::WouldBlock, 0);
    }

    // 其他 errno 值表示真正的 I/O 错误
    return std::unexpected(make_system_error());
}