#pragma once
#include "Acceptor.hpp"
#include "Channel.hpp"
#include "Connection.hpp"
#include "Poller.hpp"
#include "ThreadSafeQueue.hpp"
#include "config/Config.hpp"
#include "http/interfaces/IRequestHandler.hpp"
#include <memory>
#include <unordered_map>

class Reactor {
public:
    static auto create(Config config)
        -> std::expected<std::unique_ptr<Reactor>, std::system_error>;

    void run();

    void stop();
    void postNewConnection(Socket&& socket);

private:
    explicit Reactor(Config config);
    void onNewConnection(Socket&& socket);

    void removeConnection(int fd);

    Poller poller_;

    std::shared_ptr<http::IRequestDispatcher> http_dispatcher_;

    std::unordered_map<int, std::unique_ptr<Connection>> connections_;
    std::unordered_map<int, std::unique_ptr<Channel>> channels_;

    Config config_;
    ThreadSafeQueue<Socket> new_connections_queue_;

    std::atomic<bool> _running = true;
};