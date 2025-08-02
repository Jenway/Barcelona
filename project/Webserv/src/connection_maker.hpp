#include "Connection.hpp"
#include "Socket.hpp"
#include "TcpSinker.hpp"
#include "TcpSource.hpp"
#include "bind_to.hpp"

inline auto tag_invoke(bind_to_t /*unused*/, TcpSinker& sinker, Socket& socket)
    -> std::expected<void, std::system_error>
{
    sinker.setFd(socket.getFd());
    return {};
}

inline auto tag_invoke(bind_to_t /*unused*/, TcpSource& source, Socket& socket)
    -> std::expected<void, std::system_error>
{
    source.setFd(socket.getFd());
    return {};
}

template <typename SinkerT, typename SourceT>
auto make_connection(Socket&& socket, std::unique_ptr<protocol::IHandler> handler)
    -> std::expected<std::unique_ptr<Connection>, std::system_error>
{
    auto sinker = std::make_unique<SinkerT>();
    auto source = std::make_unique<SourceT>();

    if (auto res = bind_to(*sinker, socket); !res) {
        return std::unexpected(res.error());
    }

    if (auto res = bind_to(*source, socket); !res) {
        return std::unexpected(res.error());
    }

    return std::make_unique<Connection>(
        std::move(socket),
        std::move(handler),
        std::move(sinker),
        std::move(source));
}