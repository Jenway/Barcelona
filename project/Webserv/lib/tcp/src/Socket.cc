#include "Socket.hpp"
#include "ErrorCode.hpp"
#include "logger.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>

Socket::Socket(int fd)
    : sock_fd_(fd)
{
}

auto Socket::create() -> std::expected<Socket, std::error_code>
{
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1) {
        return std::unexpected(make_system_error());
    }
    return Socket(fd);
}

auto Socket::bind(const char* ip, uint16_t port) -> std::expected<void, std::error_code>
{
    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        return std::unexpected(ErrorCode::Net_InvalidAddress);
    }

    if (::bind(sock_fd_.get(), reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
        return std::unexpected(make_system_error());
    }
    return {};
}

auto Socket::listen(int backlog) -> std::expected<void, std::error_code>
{
    if (::listen(sock_fd_.get(), backlog) == -1) {
        return std::unexpected(make_system_error());
    }
    return {};
}

auto Socket::accept() -> std::expected<Socket, std::error_code>
{
#ifdef __linux__
    int client_fd = ::accept4(sock_fd_.get(), nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (client_fd == -1) {
        // 上层调用者会负责判断这个错误是否为 EAGAIN
        return std::unexpected(make_system_error());
    }
    return Socket(client_fd);
#else
    std::expected<int, std::error_code> result = net_utils::accept_nonblock_cloexec(sock_fd_.get());
    if (!result) {
        return std::unexpected(result.error());
    }
    return Socket(*result);
#endif
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(sock_fd_.get(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        LOG_WARN("Failed to set SO_REUSEADDR: {}", make_system_error().message());
    }
}
auto Socket::getFd() const -> int { return sock_fd_.get(); }