#pragma once
#include "IProtocolHandler.hpp"
#include "Status.hpp"

class EchoHandler : public protocol::IHandler {
private:
    std::vector<char> buffer_; // 使用双端队列作为缓冲区
    core::protocol::Status status_ { core::protocol::Status::WantRead }; // 初始状态：等待读取
    bool eof_received_ = false;

public:
    void onData(std::string_view data) override
    {
        // 将收到的数据追加到缓冲区末尾
        buffer_.insert(buffer_.end(), data.begin(), data.end());
        // 收到数据后，我们想立即写回去
        status_ = core::protocol::Status::WantWrite;
    }

    void onReadEOF() override
    {
        eof_received_ = true;
        // 如果缓冲区还有数据，继续写完。如果没有，就准备关闭。
        if (buffer_.empty()) {
            status_ = core::protocol::Status::Finished;
        } else {
            status_ = core::protocol::Status::WantWrite;
        }
    }

    auto onWriteReady(ISinker& sinker) -> std::expected<core::WriteResult, std::error_code> override
    {
        if (buffer_.empty()) {
            // 如果缓冲区是空的，有两种情况
            if (eof_received_) {
                // EOF 已经收到，并且数据已写完，可以结束了
                status_ = core::protocol::Status::Finished;
            } else {
                // 数据已写完，回到等待读取的状态
                status_ = core::protocol::Status::WantRead;
            }
            return core::WriteResult { .status = core::WriteResult::Status::Finished, .bytes_sent = 0 };
        }

        // 尝试写入缓冲区的所有数据
        // 注意：ISinker::write 接收 const char*，我们需要处理 deque 的分段内存问题
        // 为了简单起见，我们先假设可以一次性写入，或者只写入第一段
        // (一个更健壮的实现会使用 iovec 或循环写入)
        auto write_result = sinker.write(buffer_.data(), buffer_.size());

        if (write_result) {
            // 如果成功写入了部分或全部数据
            auto bytes_sent = write_result->bytes_sent;
            // 从缓冲区前面移除已发送的数据
            buffer_.erase(buffer_.begin(), buffer_.begin() + bytes_sent);
        }

        return write_result; // 直接返回 sinker 的结果
    }

    [[nodiscard]] auto getStatus() const -> core::protocol::Status override
    {
        return status_;
    }
};