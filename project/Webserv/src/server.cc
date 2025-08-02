#include "Server.hpp"
#include "Acceptor.hpp"
#include "Channel.hpp"
#include "Error.hpp"
#include "IProtocolHandler.hpp"
#include "Socket.hpp"
#include "TcpSinker.hpp"
#include "TcpSource.hpp"
#include "bind_to_impl.hpp"
#include "connection_maker.hpp"
#include "logger.hpp"
#include <csignal>
#include <memory>
#include <sys/signalfd.h>
#include <unistd.h>
#include <vector>

Server::Server(int port)
    : acceptor_([port] {
        auto acceptor_result = Acceptor::create("127.0.0.1", port);
        if (!acceptor_result) {
            throw std::system_error(acceptor_result.error());
        }
        return std::move(*acceptor_result);
    }())
{
}
Server::~Server()
{
    if (signal_fd_ != -1) {
        ::close(signal_fd_);
    }
}
void Server::setup()
{
    setupSignalHandling();

    acceptor_.setAcceptHandler([this](Socket socket) { onNewConnection(std::move(socket)); });
    acceptorChannel_ = std::make_unique<Channel>(acceptor_.getFd());
    acceptorChannel_->setReadableHandler([this] {
        acceptor_.onAccept();
    });

    if (auto res = bind_to(*acceptorChannel_, poller_); !res) {
        LOG_ERROR("Failed to bind AcceptorChannel: {}", res.error());
        std::abort();
    }
}

void Server::run()
{
    while (_running) {
        if (auto res = poller_.pollOnce(1000); !res) {
            LOG_ERROR("Poller error: {}", res.error());
            break;
        }
    }
}

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

void Server::onNewConnection(Socket&& socket)
{
    LOG_INFO("Accepted new connection: fd={}", socket.getFd());
    auto clientFd = socket.getFd();

    auto handler = std::make_unique<EchoHandler>();

    auto conn_result = make_connection<TcpSinker, TcpSource>(std::move(socket), std::move(handler));

    if (!conn_result) {
        LOG_ERROR("Failed to create connection: {}", conn_result.error());
        return;
    }

    connections_[clientFd] = std::move(*conn_result);

    auto ch = std::make_unique<Channel>(clientFd);

    Connection* conn_ptr = connections_[clientFd].get();

    auto update_poller_events = [this, conn_ptr, clientFd] {
        auto events = conn_ptr->interestedEvents();
        LOG_TRACE("fd={}: Updating poller events to (mask: {:#x})", clientFd, events);
        if (auto res = poller_.updateEvents(clientFd, events); !res) {
            LOG_ERROR("Failed to update poller events for fd={}: {}", clientFd, res.error());
            removeConnection(clientFd);
        }
    };

    ch->setReadableHandler([this, conn_ptr, clientFd, update_poller_events] {
        LOG_TRACE("Readable event on fd={}", clientFd);
        auto ret = conn_ptr->onReadable();
        if (!ret) {
            LOG_ERROR("Error on fd={} during onReadable: {}", clientFd, ret.error());
            removeConnection(clientFd);
            return;
        }
        if (conn_ptr->isClosed()) {
            LOG_INFO("Connection on fd={} is marked as closed after onReadable.", clientFd);
            removeConnection(clientFd);
            return;
        }
        update_poller_events();
    });

    ch->setWritableHandler([this, conn_ptr, clientFd, update_poller_events] {
        LOG_TRACE("Writable event on fd={}", clientFd);
        auto ret = conn_ptr->onWritable();
        if (!ret) {
            LOG_ERROR("Error on fd={} during onWritable: {}", clientFd, ret.error());
            removeConnection(clientFd);
            return;
        }
        if (conn_ptr->isClosed()) {
            LOG_INFO("Connection on fd={} is marked as closed after onWritable.", clientFd);
            removeConnection(clientFd);
            return;
        }
        update_poller_events();
    });

    if (auto res = bind_to(*ch, poller_); !res) {
        LOG_ERROR("Failed to bind Channel: {}", res.error());
        connections_.erase(clientFd);
        return;
    }

    channels_[clientFd] = std::move(ch);
}

void Server::removeConnection(int fd)
{
    connections_.erase(fd);
    channels_.erase(fd);
    LOG_INFO("Connection {} closed and removed", fd);
}

void Server::setupSignalHandling()
{
    sigset_t mask;
    sigemptyset(&mask);
    // 我们想要处理的信号
    sigaddset(&mask, SIGINT); // Ctrl+C
    sigaddset(&mask, SIGTERM); // kill 命令

    // 1. 阻塞这些信号，这样它们就不会触发默认的处理器，
    //    而是会被排队等待 signalfd 读取。
    if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
        LOG_ERROR("Failed to set sigprocmask: {}", error::to_unexpected("sigprocmask").error());
        std::abort(); // 这是一个致命的设置错误
    }

    // 2. 创建 signalfd
    signal_fd_ = ::signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (signal_fd_ == -1) {
        LOG_ERROR("Failed to create signalfd: {}", error::to_unexpected("signalfd").error());
        std::abort();
    }

    // 3. 将 signalfd 注册到 Poller
    signal_channel_ = std::make_unique<Channel>(signal_fd_);
    signal_channel_->setReadableHandler([this] {
        LOG_INFO("Caught signal, shutting down gracefully...");
        _running = false; // 停止主循环
    });

    if (auto res = bind_to(*signal_channel_, poller_); !res) {
        LOG_ERROR("Failed to bind signal_channel: {}", res.error());
        std::abort();
    }
}
