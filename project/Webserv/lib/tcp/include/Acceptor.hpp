#pragma once
#include "Socket.hpp"
#include <functional>
#include <system_error>

class Acceptor {
public:
    using AcceptHandler = std::function<void(Socket)>;

    static auto create(const char* ip, uint16_t port) -> std::expected<Acceptor, std::system_error>;
    [[nodiscard]] auto getFd() const -> int;
    // You Should not call it unless for test
    auto accept() -> std::expected<Socket, std::error_code>;

    void setAcceptHandler(AcceptHandler handler);
    void onAccept();

private:
    explicit Acceptor(Socket listen_socket);
    Socket listen_socket_;
    AcceptHandler acceptHandler_;
};