// in lib/tcp/src/Socket.cc
#include "Socket.hpp"
#include "Error.hpp"
#include "FileDescriptor.hpp"
#include "logger.hpp"
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <utility>

// --- 构造与析构 ---
Socket::Socket(int fd)
    : sock_fd_(fd)
{
}

Socket::Socket(Socket&& rhs) noexcept = default;
auto Socket::operator=(Socket&& rhs) noexcept -> Socket& = default;

void Socket::close()
{
    sock_fd_ = utils::FileDescriptor();
}

// --- 工厂函数 ---
auto Socket::create() -> std::expected<Socket, std::system_error>
{
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1) {
        return error::to_unexpected("calling socket failed");
    }
    return Socket(fd);
}

// --- 连接管理 ---
auto Socket::shutdownWrite() -> std::expected<void, std::error_code>
{
    if (!sock_fd_.isValid()) {
        return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));
    }
    if (::shutdown(sock_fd_.get(), SHUT_WR) == -1) {
        return error::to_unexpected_code();
    }
    return {};
}

// --- Socket 选项 ---
void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(sock_fd_.get(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        LOG_WARN("Failed to set SO_REUSEADDR: {}", error::to_unexpected("setsockopt").error());
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(sock_fd_.get(), IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == -1) {
        LOG_WARN("Failed to set TCP_NODELAY: {}", error::to_unexpected("setsockopt").error());
    }
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(sock_fd_.get(), SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1) {
        LOG_WARN("Failed to set SO_KEEPALIVE: {}", error::to_unexpected("setsockopt").error());
    }
}

void Socket::setLinger(bool on, int timeout_seconds)
{
    struct linger so_linger {};
    so_linger.l_onoff = on ? 1 : 0;
    so_linger.l_linger = timeout_seconds;
    if (::setsockopt(sock_fd_.get(), SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger)) == -1) {
        LOG_WARN("Failed to set SO_LINGER: {}", error::to_unexpected("setsockopt").error());
    }
}

// --- 信息查询 ---
auto Socket::getFd() const -> int { return sock_fd_.get(); }

[[nodiscard]] utils::FileDescriptor Socket::releaseFd() { return std::move(sock_fd_); }

namespace {
// 辅助函数，将 sockaddr 转换为我们的 Endpoint
auto toEndpoint(const sockaddr_storage& addr) -> std::expected<net_utils::Endpoint, std::error_code>
{
    if (addr.ss_family == AF_INET) {
        // NOLINTNEXTLINE
        const auto* ipv4 = reinterpret_cast<const sockaddr_in*>(&addr);
        char ip_str[INET_ADDRSTRLEN];
        if (::inet_ntop(AF_INET, &ipv4->sin_addr, ip_str, sizeof(ip_str)) == nullptr) {
            return error::to_unexpected_code();
        }
        return net_utils::Endpoint { .ip = ip_str, .port = ntohs(ipv4->sin_port) };
    }
    // TODO: Add IPv6 support
    return std::unexpected(std::make_error_code(std::errc::address_family_not_supported));
}
} // namespace

auto Socket::getPeerAddress() const -> std::expected<net_utils::Endpoint, std::error_code>
{
    sockaddr_storage peer_addr {};
    socklen_t addr_len = sizeof(peer_addr);
    if (::getpeername(sock_fd_.get(), reinterpret_cast<sockaddr*>(&peer_addr), &addr_len) == -1) {
        return error::to_unexpected_code();
    }
    return toEndpoint(peer_addr);
}

auto Socket::getLocalAddress() const -> std::expected<net_utils::Endpoint, std::error_code>
{
    sockaddr_storage local_addr {};
    socklen_t addr_len = sizeof(local_addr);
    if (::getsockname(sock_fd_.get(), reinterpret_cast<sockaddr*>(&local_addr), &addr_len) == -1) {
        return error::to_unexpected_code();
    }
    return toEndpoint(local_addr);
}