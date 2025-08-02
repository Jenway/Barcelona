#include "Acceptor.hpp"
#include "logger.hpp"
#include <expected>
#include <system_error>

Acceptor::Acceptor(Socket listen_socket)
    : listen_socket_(std::move(listen_socket))
{
}

auto Acceptor::create(const char* ip, uint16_t port) -> std::expected<Acceptor, std::system_error>
{
    auto socket_result = Socket::create();
    if (!socket_result) {
        return std::unexpected(socket_result.error());
    }
    Socket listen_socket = std::move(*socket_result);

    listen_socket.setReuseAddr(true);

    auto bind_result = listen_socket.bind(ip, port);
    if (!bind_result) {
        return std::unexpected(bind_result.error());
    }

    auto listen_result = listen_socket.listen();
    if (!listen_result) {
        return std::unexpected(listen_result.error());
    }

    LOG_INFO("Acceptor listening on {}:{}", ip, port);
    return Acceptor(std::move(listen_socket));
}

auto Acceptor::accept() -> std::expected<Socket, std::error_code> { return listen_socket_.accept(); }
auto Acceptor::getFd() const -> int { return listen_socket_.getFd(); }

void Acceptor::setAcceptHandler(AcceptHandler handler)
{
    acceptHandler_ = std::move(handler);
}

void Acceptor::onAccept()
{
    while (true) {
        auto clientSocketResult = listen_socket_.accept();
        if (clientSocketResult) {
            if (acceptHandler_) {
                acceptHandler_(std::move(*clientSocketResult));
            }
        } else {
            const auto& err = clientSocketResult.error();
            if (err == std::errc::resource_unavailable_try_again) {
                break;
            }
            LOG_ERROR("Accept failed: {}", err.message());
            break;
        }
    }
}