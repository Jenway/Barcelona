#include "Connection.hpp"
#include "Error.hpp"
#include "ISinker.hpp"
#include "Status.hpp"
#include "logger.hpp"
#include <csignal>
#include <magic_enum/magic_enum.hpp>
#include <poll.h>
#include <sys/epoll.h>
#include <system_error>

constexpr size_t READ_BUFFER_SIZE = 8192;

using State = core::ConnectionState;
using ReadStatus = core::ReadResult::Status;
using WriteStatus = core::WriteResult::Status;
using ProtoStatus = core::protocol::Status;
using Event = core::EventType;

Connection::Connection(Socket socket, std::unique_ptr<protocol::IHandler> handler,
    std::unique_ptr<ISource> source, std::unique_ptr<ISinker> sinker)
    : state_(State::READING)
    , socket_(std::move(socket))
    , source_(std::move(source))
    , sinker_(std::move(sinker))
    , handler_(std::move(handler))
{
    read_buffer_.resize(READ_BUFFER_SIZE);
}

auto Connection::onReadable() -> std::expected<void, std::system_error>
{
    // **关键修改 3: 处理 CLOSING 状态**
    if (state_ == State::CLOSING) {
        // 在这个状态下，我们只读取并丢弃数据，以完成 TCP 的四次挥手
        auto read_result = source_->read(read_buffer_);
        if (read_result) {
            auto [status, bytes_read] = *read_result;
            if (status == ReadStatus::Eof) {
                // 当我们读到 EOF 时，意味着客户端也关闭了写端，
                // 四次挥手完成，现在可以安全地彻底关闭套接字了。
                LOG_TRACE("fd={}: EOF received during CLOSING state. Connection fully closed.", socket_.getFd());
                state_ = State::CLOSED;
            }
        }
        // 我们不需要更新协议状态，因为协议已经结束了
        return {};
    }
    LOG_TRACE("fd={}: onReadable called.", socket_.getFd());

    auto read_result = source_->read(read_buffer_);
    if (!read_result) {
        state_ = State::CLOSED;
        return error::to_unexpected(read_result.error(), "source read failed");
    }

    auto [status, bytes_read] = *read_result;

    switch (status) {
    case ReadStatus::GotData:
        handler_->onData(std::string_view(read_buffer_.data(), bytes_read));
        break;
    case ReadStatus::Eof:
        handler_->onReadEOF();
        state_ = State::CLOSED;
        break;
    case ReadStatus::WouldBlock:
        return {};
    }

    updateStateFromProtocol();
    return {};
}
auto Connection::onWritable() -> std::expected<void, std::system_error>
{
    LOG_TRACE("fd={}: onWritable called.", socket_.getFd());

    auto write_result = handler_->onWriteReady(*sinker_);

    if (!write_result) {
        state_ = State::CLOSED;
        return error::to_unexpected(write_result.error(), "Hanlder write failed");
    }

    auto [status, bytes_write] = *write_result;

    switch (status) {
    case WriteStatus::Continue:
        state_ = State::WRITING;
        return {};
    case WriteStatus::Finished:
        break;
    }

    updateStateFromProtocol();
    return {};
}

void Connection::updateStateFromProtocol()
{
    // 如果连接已经处于关闭流程中，则不再更新状态
    if ((state_ == State::CLOSING || state_ == State::CLOSED)) {
        return;
    }
    auto old_state = state_;

    switch (handler_->getStatus()) {
    case ProtoStatus::WantRead:
        state_ = State::READING;
        break;

    case ProtoStatus::WantWrite:
        state_ = State::WRITING;
        break;
    case ProtoStatus::Finished:
        LOG_TRACE("Protocol finished on fd {}. Initiating graceful shutdown.", socket_.getFd());

        // **关键修改 1: 不再直接跳到 CLOSED**
        // 而是进入一个新的 CLOSING 状态
        state_ = State::CLOSING;

        // **立即半关闭写端**
        // 这会向客户端发送一个 FIN 包，告诉它我们不会再发送任何数据了。
        // 这给了客户端机会去读取我们刚刚发送的响应 (比如 413)。
        if (auto res = socket_.shutdownWrite(); !res) {
            LOG_WARN("Failed to shutdown write on fd {}: {}", socket_.getFd(), res.error());
            // 如果 shutdown 失败，立即强制关闭
            state_ = State::CLOSED;
        }
        break;

    case ProtoStatus::Error:
        LOG_WARN("Protocol error on fd {}. Closing connection.", socket_.getFd());
        state_ = State::CLOSED; // 协议出错，立即终止
        break;
    }
    if (old_state != state_) {
        LOG_DEBUG("fd={}: State changed from {} to {}", socket_.getFd(),
            magic_enum::enum_name(old_state), magic_enum::enum_name(state_));
    }
}

auto Connection::interestedEvents() const -> Event
{
    switch (state_) {
    case State::READING:
        return Event::Read;
    case State::WRITING:
        return Event::Write;
    case State::CLOSING:
        return Event::Read;
    case State::CLOSED:
    default:
        return Event::None;
    }
}
