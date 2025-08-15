#pragma once
#include "Endpoint.hpp"
#include "Socket.hpp"
#include <functional>
#include <system_error>

class Acceptor {
public:
    using AcceptHandler = std::function<void(Socket)>;

    static auto create(const net::TcpEndpoint& endpoint) -> std::expected<Acceptor, std::system_error>;
    [[nodiscard]] auto getFd() const -> int;
    // You Should not call it unless for test
    auto accept() -> std::expected<Socket, std::error_code>;

    void setAcceptHandler(AcceptHandler handler);
    void onAccept();

private:
    explicit Acceptor(utils::FileDescriptor listen_fd);
    utils::FileDescriptor listen_socket_;
    AcceptHandler acceptHandler_;
};