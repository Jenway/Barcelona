#include "TcpSource.hpp"
#include "ErrorCode.hpp"
#include <unistd.h> // For read()

TcpSource::TcpSource(Socket& socket)
    : socket_(socket)
{
}

auto TcpSource::read(std::vector<char>& buffer)
    -> std::expected<std::pair<core::ReadStatus, size_t>, std::error_code>
{
    ssize_t bytes_read = ::read(socket_.getFd(), buffer.data(), buffer.size());

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