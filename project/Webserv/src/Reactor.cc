// in src/Reactor.cc
#include "Reactor.hpp"
#include "Acceptor.hpp"
#include "Channel.hpp"
#include "Error.hpp"
#include "RouterBuilder.hpp"
#include "Socket.hpp"
#include "bind_to_impl.hpp"
#include "config/Config.hpp"
#include "connection_maker.hpp"
#include "http/core/HttpProtocolHandler.hpp"
#include "http/core/RequestParser.hpp"
#include "http/core/ResponseWriter.hpp"
#include <csignal>
#include <expected>
#include <memory>
#include <nlohmann/json.hpp>
#include <sys/signalfd.h>
#include <unistd.h>
#include <utility>

#define DUMMY_TAG TAG_FOR_BIND_TO_IMPL

auto Reactor::create(Config config)
    -> std::expected<std::unique_ptr<Reactor>, std::system_error>
{
    const auto& server_config = config.servers[0];
    auto reactor = std::unique_ptr<Reactor>(new Reactor(config)); // 按值传
    reactor->http_dispatcher_ = http::RouterBuilder::build(server_config);
    return reactor;
}

Reactor::Reactor(
    Config config)
    : config_(std::move(config))
{
}

void Reactor::run()
{
    while (_running) {
        std::optional<Socket> socket_opt;
        while ((socket_opt = new_connections_queue_.try_pop()).has_value()) {
            // 对于从队列中取出的每一个 socket，调用 onNewConnection
            onNewConnection(std::move(*socket_opt));
        }
        if (auto res = poller_.pollOnce(100); !res) {

            LOG_ERROR("Reactor pollOnce error: {}", res.error());
            break;
        }
    }
}

void Reactor::stop() { _running = false; }

void Reactor::postNewConnection(Socket&& socket)
{
    new_connections_queue_.push(std::move(socket));
}

void Reactor::onNewConnection(Socket&& socket)
{
    LOG_INFO("Accepted new connection: fd={}", socket.getFd());
    auto clientFd = socket.getFd();
    const auto& server_config = config_.servers[0];
    size_t max_body_size = server_config.client_max_body_size;

    auto handler = std::make_unique<http::HttpProtocolHandler>(
        std::unique_ptr<http::IRequestParser>(new http::RequestParser(max_body_size)),
        std::unique_ptr<http::IResponseWriter>(new http::ResponseWriter()),
        http_dispatcher_);

    auto conn_result = make_connection<TcpSinker, TcpSource>(std::move(socket), std::move(handler));
    if (!conn_result) {
        LOG_ERROR("Failed to create connection: {}", conn_result.error());
        return;
    }

    connections_[clientFd] = std::move(*conn_result);

    auto ch = std::make_unique<Channel>(clientFd);

    Connection* conn_ptr = connections_[clientFd].get();

    auto update_poller_events = [this, conn_ptr, clientFd] {
        auto events = conn_ptr->interestedEvents();
        LOG_TRACE("fd={}: Updating poller events to (mask: {})", clientFd, events);
        if (auto res = poller_.updateEvents(clientFd, events); !res) {
            LOG_ERROR("Failed to update poller events for fd={}: {}", clientFd, res.error());
            removeConnection(clientFd);
        }
    };

    ch->setReadableHandler([this, conn_ptr, clientFd, update_poller_events] {
        LOG_TRACE("Readable event on fd={}", clientFd);
        auto ret = conn_ptr->onReadable();
        if (!ret) {
            LOG_ERROR("Error on fd={} during onReadable: {}", clientFd, ret.error());
            removeConnection(clientFd);
            return;
        }
        if (conn_ptr->isClosed()) {
            LOG_TRACE("Connection on fd={} is marked as closed after onReadable.", clientFd);
            removeConnection(clientFd);
            return;
        }
        update_poller_events();
    });

    ch->setWritableHandler([this, conn_ptr, clientFd, update_poller_events] {
        LOG_TRACE("Writable event on fd={}", clientFd);
        auto ret = conn_ptr->onWritable();
        if (!ret) {
            LOG_ERROR("Error on fd={} during onWritable: {}", clientFd, ret.error());
            removeConnection(clientFd);
            return;
        }
        if (conn_ptr->isClosed()) {
            LOG_TRACE("Connection on fd={} is marked as closed after onWritable.", clientFd);
            removeConnection(clientFd);
            return;
        }
        update_poller_events();
    });

    if (auto res = bind_to(*ch, poller_); !res) {
        LOG_ERROR("Failed to bind Channel: {}", res.error());
        connections_.erase(clientFd);
        return;
    }

    channels_[clientFd] = std::move(ch);
}

void Reactor::removeConnection(int fd)
{
    connections_.erase(fd);
    channels_.erase(fd);
    LOG_INFO("Connection {} closed and removed", fd);
}
