#include "Connection.hpp"
#include "ErrorCode.hpp"
#include "ISinker.hpp"
#include "Status.hpp"
#include "logger.hpp"
#include <csignal>
#include <poll.h>

constexpr size_t READ_BUFFER_SIZE = 8192;

Connection::Connection(Socket socket, std::unique_ptr<protocol::IHandler> handler,
    std::unique_ptr<ISource> source, std::unique_ptr<ISinker> sinker)
    : state_(core::ConnectionState::READING)
    , socket_(std::move(socket))
    , source_(std::move(source))
    , sinker_(std::move(sinker))
    , handler_(std::move(handler))
{
    read_buffer_.resize(READ_BUFFER_SIZE);
}

auto Connection::onReadable() -> std::expected<void, std::error_code>
{
    // 将 I/O 操作完全委托给 Source
    auto read_result = source_->read(read_buffer_);
    if (!read_result) {
        state_ = core::ConnectionState::CLOSED;
        return std::unexpected(read_result.error());
    }

    auto [status, bytes_read] = *read_result;

    switch (status) {
    case core::ReadStatus::GotData:
        handler_->onData(std::string_view(read_buffer_.data(), bytes_read));
        break;
    case core::ReadStatus::Eof:
        handler_->onReadEOF();
        state_ = core::ConnectionState::CLOSED;
        break;
    case core::ReadStatus::WouldBlock:
        // 什么都不做，继续等待下一次 onReadable
        return {};
    }

    updateStateFromProtocol();
    return {};
}
auto Connection::onWritable() -> std::expected<void, std::error_code>
{
    auto write_result_opt = handler_->onWriteReady(*sinker_);

    if (!write_result_opt) {
        state_ = core::ConnectionState::CLOSED;
        return std::unexpected(write_result_opt.error());
    }

    auto write_result = *write_result_opt;

    if (write_result.status == core::WriteResult::Status::Continue) {
        // 如果只写了一部分，则保持 WRITING 状态，等待下一次 onWritable
        state_ = core::ConnectionState::WRITING;
        return {};
    }

    // 如果是 WriteStatus::Finished，则继续向下执行 updateStateFromProtocol
    updateStateFromProtocol();
    return {};
}

void Connection::updateStateFromProtocol()
{
    // 如果连接已经处于关闭流程中，则不再更新状态
    if ((state_ == core::ConnectionState::CLOSING || state_ == core::ConnectionState::CLOSED)) {
        return;
    }

    switch (handler_->getStatus()) {
    case core::protocol::Status::WantRead:
        state_ = core::ConnectionState::READING;
        break;

    case core::protocol::Status::WantWrite:
        state_ = core::ConnectionState::WRITING;
        break;
    case core::protocol::Status::Finished:
        LOG_TRACE("Protocol finished on fd {}. Closing connection.", socket_.getFd());
        state_ = core::ConnectionState::CLOSED; // 协议完成，直接关闭
        break;

    case core::protocol::Status::Error:
        LOG_WARN("Protocol error on fd {}. Closing connection.", socket_.getFd());
        state_ = core::ConnectionState::CLOSED; // 协议出错，立即终止
        break;
    }
}

auto Connection::interestedEvents() const -> uint8_t
{
    switch (state_) {
    case core::ConnectionState::READING:
        return POLL_IN;
    case core::ConnectionState::WRITING:
        return POLL_OUT;
    case core::ConnectionState::CLOSING:
    case core::ConnectionState::CLOSED:
    default:
        return 0; // 不再对任何事件感兴趣
    }
}
