#pragma once
#include "Acceptor.hpp"
#include "Channel.hpp"
#include "Connection.hpp"
#include "Poller.hpp"
#include "Reactor.hpp"
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

    int signal_fd_ = -1;
    std::unique_ptr<Channel> signal_channel_;

    Config config_;
    Acceptor acceptor_;
    Poller poller_;
    std::unique_ptr<Channel> acceptorChannel_;

    std::vector<std::unique_ptr<Reactor>> reactors_;
    std::vector<std::jthread> reactor_threads_;
    int reactor_index_ = 0;

    bool _running { true };
};
