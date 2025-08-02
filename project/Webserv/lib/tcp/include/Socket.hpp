#pragma once
#include "FileDescriptor.hpp"
#include <cstdint>
#include <expected>
#include <system_error>

class Socket {
public:
    Socket() = default;
    explicit Socket(int fd);
    Socket(Socket&) = delete;
    Socket& operator=(Socket&) = delete;
    Socket(Socket&& rhs) noexcept
        : sock_fd_(std::move(rhs.sock_fd_))
    {
    }
    auto operator=(Socket&& rhs) noexcept -> Socket&
    {
        if (this != &rhs) {
            sock_fd_ = std::move(rhs.sock_fd_);
        }
        return *this;
    }
    static auto create() -> std::expected<Socket, std::system_error>;
    auto bind(const char* ip, uint16_t port) -> std::expected<void, std::system_error>;
    auto listen(int backlog = 128) -> std::expected<void, std::system_error>;
    auto accept() -> std::expected<Socket, std::error_code>;

    void setReuseAddr(bool on);
    auto getFd() const -> int;

private:
    FileDescriptor sock_fd_;
};