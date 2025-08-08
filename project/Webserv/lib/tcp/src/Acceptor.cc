#include "Acceptor.hpp"
#include "Error.hpp"
#include "ErrorCode.hpp"
#include "logger.hpp"
#include <arpa/inet.h>
#include <expected>
#include <sys/socket.h>
#include <system_error>

Acceptor::Acceptor(utils::FileDescriptor listen_fd)
    : listen_socket_(std::move(listen_fd))
{
}

auto Acceptor::create(const char* ip, uint16_t port) -> std::expected<Acceptor, std::system_error>
{
    auto socket_result = Socket::create();
    if (!socket_result) {
        return std::unexpected(socket_result.error());
    }
    Socket temp_socket = std::move(*socket_result);
    temp_socket.setReuseAddr(true);

    const int fd = temp_socket.getFd();

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        return error::to_unexpected(ErrorCode::Net_InvalidAddress,
            fmt::format("Invalid IP address: '{}'", ip));
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): Required for C-style socket API interaction.
    if (::bind(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
        return error::to_unexpected("bind failed");
    }
    if (::listen(fd, 128) == -1) {
        return error::to_unexpected("listen failed");
    }

    LOG_INFO("Acceptor listening on {}:{}", ip, port);

    return Acceptor(temp_socket.releaseFd());
}

auto Acceptor::accept() -> std::expected<Socket, std::error_code>
{
#ifdef __linux__
    int client_fd = ::accept4(listen_socket_.get(), nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
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

auto Acceptor::getFd() const -> int { return listen_socket_.get(); }

void Acceptor::setAcceptHandler(AcceptHandler handler)
{
    acceptHandler_ = std::move(handler);
}

void Acceptor::onAccept()
{
    while (true) {
        auto clientSocketResult = accept();
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