// /lib/utils/include/fd_utils.hpp

#pragma once
#include "Error.hpp"
#include <expected>
#include <fcntl.h>
#include <system_error>

namespace utils::fd {

inline auto set_non_blocking(int fd) -> std::expected<void, std::error_code>
{
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return error::to_unexpected_code();
    }
    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return error::to_unexpected_code();
    }
    return {};
}

inline auto set_close_on_exec(int fd) -> std::expected<void, std::error_code>
{
    int flags = ::fcntl(fd, F_GETFD, 0);
    if (flags == -1) {
        return error::to_unexpected_code();
    }
    if (::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
        return error::to_unexpected_code();
    }
    return {};
}

} // namespace utils::fd