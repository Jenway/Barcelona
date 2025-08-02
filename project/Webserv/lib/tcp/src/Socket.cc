#include "Socket.hpp"
#include "Error.hpp"
#include "ErrorCode.hpp"
#include "logger.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>

Socket::Socket(int fd)
    : sock_fd_(fd)
{
}

auto Socket::create() -> std::expected<Socket, std::system_error>
{
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1) {
        return error::to_unexpected("calling socket failed");
    }
    return Socket(fd);
}

auto Socket::bind(const char* ip, uint16_t port) -> std::expected<void, std::system_error>
{
    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        return error::to_unexpected(
            ErrorCode::Net_InvalidAddress,
            fmt::format("Invalid IP address format for inet_pton: '{}'", ip));
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): Required for C-style socket API interaction.
    if (::bind(sock_fd_.get(), reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
        return error::to_unexpected("bind failed");
    }
    return {};
}

auto Socket::listen(int backlog) -> std::expected<void, std::system_error>
{
    if (::listen(sock_fd_.get(), backlog) == -1) {
        return error::to_unexpected("listen failed");
    }
    return {};
}

auto Socket::accept() -> std::expected<Socket, std::error_code>
{
#ifdef __linux__
    int client_fd = ::accept4(sock_fd_.get(), nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (client_fd == -1) {
        // 在非阻塞模式下，当没有连接时，errno 可能会被设置为 EAGAIN (System V) 或 EWOULDBLOCK (BSD)。
        // to_unexpected_code() 会将它们都捕获并转换为一个等价于
        // std::errc::resource_unavailable_try_again 的 std::error_code。
        // 上层调用者（如 Acceptor）负责处理这个特定的错误码。
        return error::to_unexpected_code();
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
        LOG_WARN("Failed to set SO_REUSEADDR: {}", error::to_unexpected("setsockopt SO_REUSEADDR failed").error());
    }
}
auto Socket::getFd() const -> int { return sock_fd_.get(); }