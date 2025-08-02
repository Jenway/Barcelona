#include "TcpSinker.hpp"

#include <cstddef>
#include <expected>
#include <unistd.h>

#include "Error.hpp"
#include "ErrorCode.hpp"
#include "net_utils.hpp"

using core::WriteResult;
using Status = core::WriteResult::Status;

auto TcpSinker::write(const char* data, size_t len)
    -> std::expected<core::WriteResult, std::error_code>
{
    if (fd_ == -1) {
        return std::unexpected(ErrorCode::Fd_NotSet);
    }

    if (len == 0) {
        return WriteResult {
            .status = Status::Finished,
            .bytes_sent = 0
        };
    }

    ssize_t write_ret = ::write(fd_, data, len);

    if (write_ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return WriteResult {
                .status = Status::Continue,
                .bytes_sent = 0
            };
        }
        return error::to_unexpected_code();
    }
    auto bytes_sent = static_cast<std::size_t>(write_ret);

    if (bytes_sent < len) {
        return WriteResult {
            .status = Status::Continue,
            .bytes_sent = bytes_sent
        };
    }

    return core::WriteResult {
        .status = Status::Finished,
        .bytes_sent = bytes_sent
    };
}

auto TcpSinker::sendfile(int in_fd, off_t& /*offset*/, size_t count)
    -> std::expected<core::WriteResult, std::error_code>
{
    if (fd_ == -1) {
        return std::unexpected(ErrorCode::Fd_NotSet);
    }
    if (count == 0) {
        return core::WriteResult { .status = core::WriteResult::Status::Finished, .bytes_sent = 0 };
    }

    auto result = net_utils::sendfile(fd_, in_fd, nullptr, count);

    if (!result) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return WriteResult {
                .status = Status::Continue,
                .bytes_sent = 0
            };
        }
        return std::unexpected(result.error());
    }

    std::size_t bytes_sent = *result;

    if (bytes_sent < count) {
        return core::WriteResult {
            .status = Status::Continue,
            .bytes_sent = bytes_sent
        };
    }

    return core::WriteResult {
        .status = Status::Finished,
        .bytes_sent = bytes_sent
    };
}