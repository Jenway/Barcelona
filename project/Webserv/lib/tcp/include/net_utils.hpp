#pragma once
#include "ErrorCode.hpp"
#include "fd_utils.hpp"
#include <expected>
#include <fcntl.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

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

} // namespace net_utils