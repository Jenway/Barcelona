#pragma once
#include "Acceptor.hpp"
#include "Channel.hpp"
#include "Connection.hpp"
#include "Poller.hpp"

#include <memory>
#include <unordered_map>

class Server {
public:
    explicit Server(int port);
    ~Server();
    void setup();
    void run();

private:
    void onNewConnection(Socket&& socket);
    void removeConnection(int fd);
    void setupSignalHandling();

    int signal_fd_ = -1;
    std::unique_ptr<Channel> signal_channel_;
    Acceptor acceptor_;
    Poller poller_;
    std::unique_ptr<Channel> acceptorChannel_;
    std::unordered_map<int, std::unique_ptr<Connection>> connections_;
    std::unordered_map<int, std::unique_ptr<Channel>> channels_;
    bool _running { true };
};
