#include "Acceptor.hpp"
#include "ErrorCode.hpp"
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>

// 测试套件：TcpComponentsTest
// 目标：验证 TCP 底层组件的行为

struct ThreadJoiner {
    std::thread& t;
    explicit ThreadJoiner(std::thread& thread_to_join)
        : t(thread_to_join)
    {
    }
    ~ThreadJoiner()
    {
        if (t.joinable()) {
            t.join();
        }
    }
};

// 测试套件：TcpComponentsTest
TEST(TcpComponentsTest, AcceptorLifecycle)
{
    const char* ip = "127.0.0.1";
    constexpr uint16_t port = 8081;

    auto acceptor_result = Acceptor::create(ip, port);
    ASSERT_TRUE(acceptor_result.has_value()) << "Failed to create acceptor: " << acceptor_result.error().message();
    Acceptor acceptor = std::move(*acceptor_result);

    std::thread client_thread;
    ThreadJoiner joiner(client_thread);

    client_thread = std::thread([&]() {
        // 为了简化测试，我们直接创建一个标准的、阻塞的 socket
        int client_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        ASSERT_NE(client_fd, -1);
        FileDescriptor fd_guard(client_fd); // 用 FileDescriptor 来管理它的生命周期
        // ===============================================================================

        // 延迟一小会儿，确保服务器的 accept() 调用已经准备就绪
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        sockaddr_in server_addr {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(ip);

        // 在阻塞模式下，这个 connect 会等待连接完成
        int ret = ::connect(client_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
        ASSERT_NE(ret, -1) << "Client failed to connect: " << strerror(errno);
    });

    // 服务器端现在应该能成功 accept() 了
    auto client_result = acceptor.accept();

    // 如果第一次失败（因为时序问题），我们再试一次
    if (!client_result.has_value() && client_result.error() == std::errc::resource_unavailable_try_again) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 等待更长的时间
        client_result = acceptor.accept();
    }

    ASSERT_TRUE(client_result.has_value()) << "Accept failed: " << client_result.error().message();
    Socket client_connection = std::move(*client_result);
    EXPECT_TRUE(client_connection.getFd() > 0);
}

// 测试 2: 验证创建 Acceptor 时的应用级错误（无效 IP）
TEST(TcpComponentsTest, InvalidAddressError)
{
    const char* invalid_ip = "999.999.999.999";
    auto acceptor_result = Acceptor::create(invalid_ip, 8082);

    // 断言操作失败
    ASSERT_FALSE(acceptor_result.has_value());

    // 断言返回的错误是我们自定义的、类型安全的 Net_InvalidAddress！
    EXPECT_EQ(acceptor_result.error(), ErrorCode::Net_InvalidAddress);
}

// 测试 3: 验证非阻塞 accept 在没有连接时的行为
TEST(TcpComponentsTest, AcceptReturnsEagain)
{
    auto acceptor_result = Acceptor::create("127.0.0.1", 8083);
    ASSERT_TRUE(acceptor_result.has_value());
    Acceptor acceptor = std::move(*acceptor_result);

    auto client_result = acceptor.accept();

    ASSERT_FALSE(client_result.has_value());

    EXPECT_EQ(client_result.error(), std::errc::resource_unavailable_try_again);
}