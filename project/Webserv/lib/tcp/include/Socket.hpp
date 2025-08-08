// in lib/tcp/include/Socket.hpp
#pragma once
#include "FileDescriptor.hpp"
#include <cstdint>
#include <expected>
#include <string>
#include <system_error>

namespace net_utils {
struct Endpoint {
    std::string ip;
    uint16_t port;
};
} // namespace net_utils

class Socket {
public:
    // --- 构造与生命周期 ---
    Socket() = default;
    explicit Socket(int fd);
    Socket(Socket&) = delete;
    Socket& operator=(Socket&) = delete;
    Socket(Socket&&) noexcept;
    auto operator=(Socket&&) noexcept -> Socket&;
    ~Socket() = default;

    static auto create() -> std::expected<Socket, std::system_error>;
    void close();

    // --- 连接管理 ---
    auto shutdownWrite() -> std::expected<void, std::error_code>;

    // --- Socket 选项 ---
    void setReuseAddr(bool on);
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);
    void setLinger(bool on, int timeout_seconds);

    // --- 信息查询 ---
    [[nodiscard]] auto getFd() const -> int;
    [[nodiscard]] auto getPeerAddress() const -> std::expected<net_utils::Endpoint, std::error_code>;
    [[nodiscard]] auto getLocalAddress() const -> std::expected<net_utils::Endpoint, std::error_code>;

    [[nodiscard]] utils::FileDescriptor releaseFd();

private:
    utils::FileDescriptor sock_fd_;
};