#pragma once

#include <memory>
#include <system_error>
#include <vector>

#include "IProtocolHandler.hpp"
#include "ISinker.hpp"
#include "ISource.hpp"
#include "Socket.hpp"
#include "Status.hpp"

class Connection {
public:
    Connection(Socket socket, std::unique_ptr<protocol::IHandler> handler,
        std::unique_ptr<ISource> source, std::unique_ptr<ISinker> sinker);

    auto onReadable() -> std::expected<void, std::error_code>;

    auto onWritable() -> std::expected<void, std::error_code>;
    [[nodiscard]] auto isClosed() const -> bool { return state_ == core::ConnectionState::CLOSED; }

    [[nodiscard]] auto interestedEvents() const -> uint8_t;

    [[nodiscard]] auto toString() const -> std::string;

private:
    void updateStateFromProtocol();

    core::ConnectionState state_;

    Socket socket_;
    std::unique_ptr<ISource> source_;
    std::unique_ptr<ISinker> sinker_;
    std::unique_ptr<protocol::IHandler> handler_;
    ResponsePtr response_to_send_;
    std::vector<char> read_buffer_;
};
