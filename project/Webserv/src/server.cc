#include "Server.hpp"
#include "Acceptor.hpp"
#include "Channel.hpp"
#include "Error.hpp"
#include "Socket.hpp"
#include "bind_to_impl.hpp"
#include "config/Config.hpp"
#include "connection_maker.hpp"
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
    auto acceptor_result = Acceptor::create({ "127.0.0.1", static_cast<uint16_t>(port) });
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

constexpr int kReactorCount = 4;
auto Server::setup() -> std::expected<void, std::system_error>
{
    setupSignalHandling();

    const auto& server_config = config_.servers[0];

    LOG_INFO("Setting up HTTP request handlers for server '{}'...", server_config.server_name);

    acceptor_.setAcceptHandler([this](Socket socket) { onNewConnection(std::move(socket)); });
    acceptorChannel_ = std::make_unique<Channel>(acceptor_.getFd());
    acceptorChannel_->setReadableHandler([this] {
        acceptor_.onAccept();
    });

    if (auto res = bind_to(*acceptorChannel_, poller_); !res) {
        LOG_ERROR("Failed to bind AcceptorChannel: {}", res.error());
        std::abort();
    }
    for (int i = 0; i < kReactorCount; ++i) {
        auto r = Reactor::create(config_);
        if (!r) {
            return std::unexpected(r.error());
        }
        reactors_.emplace_back(std::move(*r));
    }
    return {};
}

void Server::run()
{
    reactor_threads_.clear();
    reactor_threads_.reserve(reactors_.size());
    for (auto& reactor : reactors_) {
        Reactor* r = reactor.get();
        reactor_threads_.emplace_back([r] { r->run(); });
    }

    LOG_INFO("Server is running... Press Ctrl+C to stop.");
    while (_running) {
        if (auto res = poller_.pollOnce(1000); !res) {
            LOG_ERROR("Poller error: {}", res.error());
            break;
        }
    }
    for (auto& r : reactors_)
        r->stop();
}
void Server::onNewConnection(Socket&& socket)
{
    LOG_INFO("Accepted new connection: fd={}", socket.getFd());
    static int reactor_index = 0;

    if (reactors_.empty()) {
        LOG_ERROR("No reactors available to handle new connections.");
        return;
    }

    auto& reactor = reactors_[reactor_index];
    reactor->postNewConnection(std::move(socket));

    // Round-robin to the next reactor
    reactor_index = (reactor_index + 1) % reactors_.size();
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
        for (auto& r : reactors_)
            r->stop();
    });

    if (auto res = bind_to(*signal_channel_, poller_); !res) {
        auto err = error::to_unexpected("Failed to bind signal_channel").error();
        return std::unexpected(err);
    }
    return {};
}
