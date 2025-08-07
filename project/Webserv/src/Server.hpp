#pragma once
#include "Acceptor.hpp"
#include "Channel.hpp"
#include "Connection.hpp"
#include "Poller.hpp"
#include "config/Config.hpp"
#include "http/interfaces/IRequestHandler.hpp"
#include <memory>
#include <system_error>
#include <unordered_map>

class Server {
public:
    static auto create(Config config) -> std::expected<std::unique_ptr<Server>, std::system_error>;

    ~Server();
    void run();

private:
    Server(Config cfg, Acceptor acceptor);

    auto setup() -> std::expected<void, std::system_error>;
    auto setupSignalHandling() -> std::expected<void, std::system_error>;

    void onNewConnection(Socket&& socket);
    void removeConnection(int fd);

    int signal_fd_ = -1;
    std::unique_ptr<Channel> signal_channel_;

    Config config_;
    Acceptor acceptor_;
    Poller poller_;
    std::unique_ptr<Channel> acceptorChannel_;

    std::shared_ptr<http::IRequestDispatcher> _http_dispatcher;

    std::unordered_map<int, std::unique_ptr<Connection>> connections_;
    std::unordered_map<int, std::unique_ptr<Channel>> channels_;
    bool _running { true };
};
