#include <arpa/inet.h>
#include <atomic>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Acceptor.hpp"
#include "Socket.hpp"

class AcceptorTest : public ::testing::Test {
protected:
    std::unique_ptr<Acceptor> acceptor_;
    uint16_t listening_port_ = 0; // The ephemeral port our acceptor is listening on

    // 在每个测试用例开始前，创建一个真实的 Acceptor
    void SetUp() override
    {
        // 使用端口 0，让操作系统为我们选择一个可用的临时端口
        auto acceptor_result = Acceptor::create("127.0.0.1", 0);
        // 如果 Acceptor 创建失败，测试将直接失败
        ASSERT_TRUE(acceptor_result.has_value()) << "Failed to create Acceptor: " << acceptor_result.error().what();
        acceptor_ = std::make_unique<Acceptor>(std::move(*acceptor_result));

        // 获取 Acceptor 实际监听的端口，以便客户端可以连接
        sockaddr_in addr {};
        socklen_t len = sizeof(addr);
        ASSERT_EQ(getsockname(acceptor_->getFd(), (struct sockaddr*)&addr, &len), 0);
        listening_port_ = ntohs(addr.sin_port);
        ASSERT_GT(listening_port_, 0); // 确认我们得到了一个有效的端口
    }

    void TearDown() override
    {
        // unique_ptr 会自动销毁 Acceptor，从而关闭监听套接字
    }

    // 辅助函数：创建一个连接到我们 acceptor 的客户端套接字
    // 返回一个有效的文件描述符，如果失败则返回 -1
    int create_client_and_connect()
    {
        int client_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0)
            return -1;

        sockaddr_in server_addr {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(listening_port_);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (::connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            ::close(client_fd);
            return -1;
        }
        return client_fd;
    }
};

TEST_F(AcceptorTest, CallsAcceptHandlerOnNewConnection)
{
    std::atomic<bool> handler_was_called = false;
    std::atomic<int> accepted_fd = -1;

    acceptor_->setAcceptHandler([&](Socket client_socket) {
        handler_was_called = true;
        accepted_fd = client_socket.getFd();
    });

    int client_fd = create_client_and_connect();
    ASSERT_NE(client_fd, -1);

    // *** 修复点: 增加短暂延迟 ***
    // 给内核一点时间来处理连接并将其放入 accept 队列
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    acceptor_->onAccept();

    ASSERT_TRUE(handler_was_called);
    ASSERT_NE(accepted_fd, -1);

    ::close(client_fd);
}

TEST_F(AcceptorTest, HandlesMultiplePendingConnections)
{
    std::atomic<int> call_count = 0;

    acceptor_->setAcceptHandler([&](Socket /*client_socket*/) {
        call_count++;
    });

    int client1 = create_client_and_connect();
    int client2 = create_client_and_connect();
    ASSERT_NE(client1, -1);
    ASSERT_NE(client2, -1);

    // *** 修复点: 增加短暂延迟 ***
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    acceptor_->onAccept();

    ASSERT_EQ(call_count, 2);

    ::close(client1);
    ::close(client2);
}

// 测试：当没有新连接时，onAccept 不应调用回调函数
TEST_F(AcceptorTest, DoesNothingWhenNoConnections)
{
    std::atomic<bool> handler_was_called = false;

    // Arrange: 设置一个如果被调用就会导致测试失败的回调
    acceptor_->setAcceptHandler([&](Socket /*client_socket*/) {
        handler_was_called = true;
        FAIL() << "Accept handler should not be called when there are no connections.";
    });

    // Act: 在没有任何客户端连接的情况下直接调用 onAccept
    acceptor_->onAccept();

    // Assert: 确认回调没有被调用
    ASSERT_FALSE(handler_was_called);
}