#include "TcpSource.hpp"

#include <cstddef>
#include <unistd.h>

#include "Error.hpp"
#include "ErrorCode.hpp"

using core::ReadResult;
using Status = core::ReadResult::Status;

auto TcpSource::read(std::vector<char>& buffer)
    -> std::expected<core::ReadResult, std::error_code>
{
    if (fd_ == -1) {
        return std::unexpected(ErrorCode::Fd_NotSet);
    }

    ssize_t read_ret = ::read(fd_, buffer.data(), buffer.size());

    if (read_ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return ReadResult {
                .status = Status::WouldBlock,
                .bytes_read = 0
            };
        }
        return error::to_unexpected_code();
    }

    auto bytes_read = static_cast<std::size_t>(read_ret);

    if (bytes_read == 0) {
        return ReadResult {
            .status = Status::Eof,
            .bytes_read = 0
        };
    }

    return ReadResult {
        .status = Status::GotData,
        .bytes_read = bytes_read
    };
}