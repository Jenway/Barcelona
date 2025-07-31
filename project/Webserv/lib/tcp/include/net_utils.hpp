#pragma once
#include "ErrorCode.hpp"
#include "fd_utils.hpp"
#include <expected>
#include <fcntl.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

#ifdef __linux__
#include <sys/sendfile.h>
#endif

namespace net_utils {

inline auto accept_nonblock_cloexec(int listen_fd) -> std::expected<int, std::error_code>
{
    int client_fd = ::accept(listen_fd, nullptr, nullptr);
    if (client_fd == -1) {
        return std::unexpected(make_system_error());
    }

    if (auto res = utils::fd::set_non_blocking(client_fd); !res) {
        ::close(client_fd);
        return std::unexpected(res.error());
    }
    if (auto res = utils::fd::set_close_on_exec(client_fd); !res) {
        ::close(client_fd);
        return std::unexpected(res.error());
    }

    return client_fd;
}

inline auto sendfile(int out_fd, int in_fd, off_t* offset, size_t count)
    -> std::expected<ssize_t, std::error_code>
{
    ssize_t bytes_sent = ::sendfile(out_fd, in_fd, offset, count);

    if (bytes_sent == -1) {
        return std::unexpected(make_system_error());
    }

    return bytes_sent;
}

} // namespace net_utils