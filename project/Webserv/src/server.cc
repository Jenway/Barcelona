#include "Server.hpp"
#include "Acceptor.hpp"
#include "Channel.hpp"
#include "IProtocolHandler.hpp"
#include "Socket.hpp"
#include "TcpSinker.hpp"
#include "TcpSource.hpp"
#include "bind_to_impl.hpp"
#include "connection_maker.hpp"
#include "logger.hpp"
#include <memory>

Server::Server(int port)
    : acceptor_(*Acceptor::create("127.0.0.1", port))
{
}

void Server::setup()
{
    acceptor_.setAcceptHandler([this](Socket socket) { onNewConnection(std::move(socket)); });
    acceptorChannel_ = std::make_unique<Channel>(acceptor_.getFd());
    acceptorChannel_->setReadableHandler([this] {
        acceptor_.onAccept();
    });

    if (auto res = bind_to(*acceptorChannel_, poller_); !res) {
        LOG_ERROR("Failed to bind AcceptorChannel: {}", res.error().what());
    }
}

void Server::run()
{
    while (_running) {
        if (auto res = poller_.pollOnce(1000); !res) {
            LOG_ERROR("Poller error: {}", res.error().what());
            break;
        }
    }
}

class dummyHandler : public protocol::IHandler {
public:
    void onData(std::string_view data) { };
    void onReadEOF() { };

    auto onWriteReady(ISinker& sinker)
        -> std::expected<core::WriteResult, std::error_code> { };

    [[nodiscard]] auto getStatus() const -> core::protocol::Status { };
};

void Server::onNewConnection(Socket&& socket)
{
    LOG_INFO("Accepted new connection: fd={}", socket.getFd());
    auto clientFd = socket.getFd();

    auto handler = std::make_unique<dummyHandler>();

    auto conn_result = make_connection<TcpSinker, TcpSource>(std::move(socket), std::move(handler));

    if (!conn_result) {
        LOG_ERROR("Failed to create connection: {}", conn_result.error().what());
        return;
    }

    connections_[clientFd] = std::move(*conn_result);

    auto ch = std::make_unique<Channel>(clientFd);

    Connection* conn_ptr = connections_[clientFd].get();

    ch->setReadableHandler([conn_ptr] {
        conn_ptr->onReadable();
    });
    ch->setWritableHandler([conn_ptr] {
        conn_ptr->onWritable();
    });

    if (auto res = bind_to(*ch, poller_); !res) {
        LOG_ERROR("Failed to bind Channel: {}", res.error().what());
        connections_.erase(clientFd);
        return;
    }

    channels_[clientFd] = std::move(ch);
}

void Server::removeConnection(int fd)
{
    connections_.erase(fd);
    channels_.erase(fd);
    LOG_INFO("Connection %d closed and removed", fd);
}
