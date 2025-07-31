#pragma once
#include "Socket.hpp"
#include <system_error>

class Acceptor {
public:
    static auto create(const char* ip, uint16_t port) -> std::expected<Acceptor, std::error_code>;
    auto accept() -> std::expected<Socket, std::error_code>;
    [[nodiscard]] auto getFd() const -> int;

private:
    explicit Acceptor(Socket listen_socket);
    Socket listen_socket_;
};