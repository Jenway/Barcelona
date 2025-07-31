#pragma once
#include "FileDescriptor.hpp"
#include <cstdint>
#include <expected>
#include <system_error>

class Socket {
public:
    Socket() = default;
    explicit Socket(int fd);

    static auto create() -> std::expected<Socket, std::error_code>;
    auto bind(const char* ip, uint16_t port) -> std::expected<void, std::error_code>;
    auto listen(int backlog = 128) -> std::expected<void, std::error_code>;
    auto accept() -> std::expected<Socket, std::error_code>;

    void setReuseAddr(bool on);
    auto getFd() const -> int;

private:
    FileDescriptor sock_fd_;
};