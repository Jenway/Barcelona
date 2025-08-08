#include "Acceptor.hpp"
#include "Connection.hpp"
#include "IProtocolHandler.hpp"
#include "Socket.hpp"
#include "Status.hpp"
#include "TcpSinker.hpp"
#include "TcpSource.hpp"
#include <arpa/inet.h>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <thread>

using namespace ::testing;

// --- Mock Handler for Integration Test ---
class MockIntegrationHandler : public protocol::IHandler {
public:
    // 我们用 Promise/Future 来在线程间同步数据
    std::promise<std::string> data_promise;
    std::string data_to_send = "pong";

    MockIntegrationHandler()
        : current_status_(core::protocol::Status::WantRead)
    {
    }

    void onData(std::string_view data) override
    {
        // 收到数据后，将其设置到 promise 中，并切换状态
        data_promise.set_value(std::string(data));
        current_status_ = core::protocol::Status::WantWrite;
    }

    void onReadEOF() override { }

    auto onWriteReady(ISinker& sinker)
        -> std::expected<core::WriteResult, std::error_code> override
    {
        // 收到 onWriteReady 事件后，发送 "pong"
        auto result = sinker.write(data_to_send.data(), data_to_send.size());
        if (result && result->status == core::WriteResult::Status::Finished) {
            current_status_ = core::protocol::Status::Finished;
        }
        return result;
    }

    [[nodiscard]] auto getStatus() const -> core::protocol::Status override
    {
        return current_status_;
    }

private:
    core::protocol::Status current_status_;
};

// --- 集成测试 ---
TEST(TcpIntegrationTest, FullPingPongCycle)
{
    // ====================== 关键改动 1: 动态端口分配 ======================
    const char* ip = "127.0.0.1";
    // 绑定端口 0，让 OS 自动选择一个可用端口
    auto acceptor_result = Acceptor::create(ip, 0);
    ASSERT_TRUE(acceptor_result.has_value()) << "Acceptor::create failed: " << acceptor_result.error().what();
    Acceptor acceptor = std::move(*acceptor_result);

    // 获取 OS 分配的实际端口
    sockaddr_in addr {};
    socklen_t len = sizeof(addr);
    ASSERT_NE(getsockname(acceptor.getFd(), (struct sockaddr*)&addr, &len), -1);
    uint16_t actual_port = ntohs(addr.sin_port);
    ASSERT_GT(actual_port, 0);

    std::promise<Socket> server_socket_promise;
    std::future<Socket> server_socket_future = server_socket_promise.get_future();

    std::thread server_thread([&]() {
        auto server_socket_result = acceptor.accept();
        while (!server_socket_result.has_value()) {
            if (server_socket_result.error() != std::errc::resource_unavailable_try_again) {
                // 如果是致命错误，让 promise 失败
                server_socket_promise.set_exception(
                    std::make_exception_ptr(std::runtime_error("Accept failed in test")));
                return;
            }
            server_socket_result = acceptor.accept();
        }
        // 连接成功，满足 promise
        server_socket_promise.set_value(std::move(*server_socket_result));
    });

    // 2. 客户端连接到 OS 分配的端口
    auto client_socket_result = Socket::create();
    ASSERT_TRUE(client_socket_result.has_value());
    Socket client_socket = std::move(*client_socket_result);

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(actual_port); // 使用动态端口
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // 重试连接
    int ret = -1;
    for (int i = 0; i < 100 && ret == -1; ++i) { // Retry up to 1 second
        ret = ::connect(client_socket.getFd(), (sockaddr*)&server_addr, sizeof(server_addr));
        if (ret == -1 && errno != EINPROGRESS && errno != ECONNREFUSED) {
            FAIL() << "Client connect failed: " << strerror(errno);
        }
        if (ret == 0)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_EQ(ret, 0) << "Client failed to connect after retries";

    // 3. 等待服务器线程完成 accept，最多等待1秒
    ASSERT_EQ(server_socket_future.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    Socket accepted_socket = server_socket_future.get(); // 得到一个栈上的 Socket
    server_thread.join();

    // 4. 创建完整的服务器端 Connection
    auto handler = std::make_unique<MockIntegrationHandler>();
    auto handler_future = handler->data_promise.get_future();

    auto source = std::make_unique<TcpSource>();
    auto sinker = std::make_unique<TcpSinker>();
    source->setFd(accepted_socket.getFd());
    sinker->setFd(accepted_socket.getFd());

    Connection connection(std::move(accepted_socket), std::move(handler),
        std::move(source), std::move(sinker));

    // 5. 客户端发送 "ping"
    const std::string ping_msg = "ping";
    ASSERT_EQ(::write(client_socket.getFd(), ping_msg.data(), ping_msg.size()), ping_msg.size());

    // 6. 触发服务器端 onReadable，并加入重试逻辑
    //    这可以处理客户端 write() 和服务器端 read() 之间的时序问题
    std::expected<void, std::system_error> read_res;
    bool read_success = false;
    for (int i = 0; i < 100; ++i) { // Retry up to 1 second
        read_res = connection.onReadable();
        // 我们只关心 handler 是否收到了数据
        if (handler_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            read_success = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ASSERT_TRUE(read_success) << "Server failed to receive data from client.";
    ASSERT_TRUE(read_res.has_value());

    // 7. 验证 Handler 是否收到了 "ping"
    EXPECT_EQ(handler_future.get(), ping_msg);

    // 8. 此时 Connection 应该想写了
    ASSERT_EQ(connection.interestedEvents(), core::EventType::Write);

    // 9. 触发服务器端 onWritable
    auto write_res = connection.onWritable();
    ASSERT_TRUE(write_res.has_value());

    // 10. 验证客户端是否收到了 "pong"
    //     同样加入重试逻辑，因为网络传输不是瞬时的
    char buffer[1024];
    ssize_t bytes_read = -1;
    for (int i = 0; i < 100 && bytes_read <= 0; ++i) {
        bytes_read = ::read(client_socket.getFd(), buffer, sizeof(buffer));
        if (bytes_read > 0)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ASSERT_GT(bytes_read, 0);
    EXPECT_EQ(std::string_view(buffer, bytes_read), "pong");
    ::shutdown(client_socket.getFd(), SHUT_WR);

    // **新增：12. 再次触发 onReadable，让服务器处理 EOF**
    //    服务器在 CLOSING 状态下，收到读事件 (EOF) 后，
    //    就会将自己的状态切换到 CLOSED。
    //    我们需要给网络和事件循环一点时间来传递这个 EOF 事件。
    bool server_closed = false;
    for (int i = 0; i < 100; ++i) { // Retry up to 1 second
        connection.onReadable(); // 触发服务器端的 EOF 处理
        if (connection.isClosed()) {
            server_closed = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_TRUE(server_closed) << "Connection did not transition to CLOSED after client shutdown.";

    // 11. 此时 Connection 应该完成了
    ASSERT_TRUE(connection.isClosed());
}