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
    if (!response_to_send_) {
        LOG_WARN("onWritable called but no response to send for fd {}", socket_.getFd());
        // 这种情况下，通常我们应该重新对读事件感兴趣
        state_ = core::ConnectionState::READING;
        return {};
    }

    // 将响应交给 Sinker 去发送
    auto sink_result = sinker_->send(*response_to_send_);
    if (!sink_result) {
        state_ = core::ConnectionState::CLOSED; // Sinker 发生错误，关闭连接
        return std::unexpected(sink_result.error());
    }

    // 根据 Sinker 的发送结果来决定下一步
    switch (*sink_result) {
    case core::WriteStatus::Finished:
        // 响应已完全发送，清空待发送的响应
        response_to_send_.reset();
        break;
    case core::WriteStatus::Continue:
        // 响应只发送了一部分，保持 WRITING 状态，等待下一次 onWritable
        state_ = core::ConnectionState::WRITING;
        return {}; // 保持 WRITING 状态
    case core::WriteStatus::Error:
        // 不应该走到这里，因为错误会通过 expected 返回
        state_ = core::ConnectionState::CLOSED;
        return std::unexpected(make_error_code(ErrorCode::Unknown));
    }

    // 如果发送完成，更新状态机
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
        // 协议说它想写了，我们就去问它要“响应”
        response_to_send_ = handler_->produceResponse();
        if (response_to_send_) {
            // 如果成功拿到了响应，我们就切换到 WRITING 状态
            state_ = core::ConnectionState::WRITING;
        } else {
            // 如果拿不到响应（可能协议还在处理中），我们继续保持 READING 状态
            state_ = core::ConnectionState::READING;
            LOG_DEBUG("Protocol wants to write, but no response produced yet for fd {}",
                socket_.getFd());
        }
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
