#include "Server.hpp"
#include "Acceptor.hpp"
#include "Channel.hpp"
#include "Error.hpp"
#include "IProtocolHandler.hpp"
#include "Socket.hpp"
#include "TcpSinker.hpp"
#include "TcpSource.hpp"
#include "bind_to_impl.hpp"
#include "config/Config.hpp"
#include "connection_maker.hpp"
#include "http/core/HttpProtocolHandler.hpp"
#include "http/core/RequestParser.hpp"
#include "http/core/ResponseWriter.hpp"
#include "http/routing/RouterBuilder.hpp"
#include "logger.hpp"
#include <csignal>
#include <expected>
#include <memory>
#include <nlohmann/json.hpp>
#include <sys/signalfd.h>
#include <system_error>
#include <unistd.h>
#include <vector>

auto Server::create(Config config) -> std::expected<std::unique_ptr<Server>, std::system_error>
{

    if (config.servers.empty()) {
        return error::to_unexpected(std::errc::invalid_argument, "No server blocks found in configuration.");
    }

    int port = config.servers[0].listen;
    auto acceptor_result = Acceptor::create("127.0.0.1", port);
    if (!acceptor_result) {
        return std::unexpected(acceptor_result.error());
    }

    auto server = std::unique_ptr<Server>(new Server(std::move(config), std::move(*acceptor_result)));

    if (auto res = server->setup(); !res) {
        return std::unexpected(res.error());
    }

    LOG_INFO("Server created and setup successfully. Ready to run.");
    return server;
}

Server::Server(Config cfg, Acceptor acceptor)
    : config_(std::move(cfg))
    , acceptor_(std::move(acceptor))
{
}

Server::~Server()
{
    if (signal_fd_ != -1) {
        ::close(signal_fd_);
    }
}

auto Server::setup() -> std::expected<void, std::system_error>
{
    setupSignalHandling();

    const auto& server_config = config_.servers[0];

    LOG_INFO("Setting up HTTP request handlers for server '{}'...", server_config.server_name);
    _http_dispatcher = http::RouterBuilder::build(server_config);

    acceptor_.setAcceptHandler([this](Socket socket) { onNewConnection(std::move(socket)); });
    acceptorChannel_ = std::make_unique<Channel>(acceptor_.getFd());
    acceptorChannel_->setReadableHandler([this] {
        acceptor_.onAccept();
    });

    if (auto res = bind_to(*acceptorChannel_, poller_); !res) {
        LOG_ERROR("Failed to bind AcceptorChannel: {}", res.error());
        std::abort();
    }
    return {};
}

void Server::run()
{
    while (_running) {
        if (auto res = poller_.pollOnce(1000); !res) {
            LOG_ERROR("Poller error: {}", res.error());
            break;
        }
    }
}

void Server::onNewConnection(Socket&& socket)
{
    LOG_INFO("Accepted new connection: fd={}", socket.getFd());
    auto clientFd = socket.getFd();

    auto handler = std::make_unique<http::HttpProtocolHandler>(
        std::unique_ptr<http::IRequestParser>(new http::RequestParser()),
        std::unique_ptr<http::IResponseWriter>(new http::ResponseWriter()),
        _http_dispatcher);

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

void Server::removeConnection(int fd)
{
    connections_.erase(fd);
    channels_.erase(fd);
    LOG_INFO("Connection {} closed and removed", fd);
}

auto Server::setupSignalHandling() -> std::expected<void, std::system_error>
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT); // Ctrl+C
    sigaddset(&mask, SIGTERM); // kill 命令

    // 1. 阻塞这些信号，这样它们就不会触发默认的处理器，
    //    而是会被排队等待 signalfd 读取。
    if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
        auto err = error::to_unexpected("Failed to set sigprocmask").error();
        return std::unexpected(err);
    }

    // 2. 创建 signalfd
    signal_fd_ = ::signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (signal_fd_ == -1) {
        auto err = error::to_unexpected("Failed to create signalfd").error();
        return std::unexpected(err);
    }

    // 3. 将 signalfd 注册到 Poller
    signal_channel_ = std::make_unique<Channel>(signal_fd_);
    signal_channel_->setReadableHandler([this] {
        LOG_INFO("Caught signal, shutting down gracefully...");
        _running = false;
    });

    if (auto res = bind_to(*signal_channel_, poller_); !res) {
        auto err = error::to_unexpected("Failed to bind signal_channel").error();
        return std::unexpected(err);
    }
    return {};
}
