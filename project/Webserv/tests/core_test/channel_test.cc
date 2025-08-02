#include "Channel.hpp"
#include "bind_to.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/epoll.h>
#include <system_error>

// GMock 无法直接模拟返回 std::expected 的函数
// 所以我们创建一个基类接口让 MockPoller 继承
class IPoller {
public:
    virtual ~IPoller() = default;
    virtual std::expected<void, std::system_error> addFd(int fd) = 0;
    virtual std::expected<void, std::system_error> registerCallback(int fd, uint32_t eventMask, std::function<void()> cb) = 0;
};

// 创建一个 MockPoller 来模拟 Poller 的行为
class MockPoller : public IPoller {
public:
    MOCK_METHOD((std::expected<void, std::system_error>), addFd, (int fd), (override));
    MOCK_METHOD((std::expected<void, std::system_error>), registerCallback, (int fd, uint32_t eventMask, std::function<void()> cb), (override));
};

// 由于我们的 bind_to 实现是针对具体类型 Poller& 的，
// 我们创建一个适配器，使其能与我们的 Mock 一起工作。
// 这也展示了 bind_to 模式的灵活性。
struct MockPollerAdapter {
    MockPoller& mock;

    std::expected<void, std::system_error> addFd(int fd)
    {
        return mock.addFd(fd);
    }

    std::expected<void, std::system_error> registerCallback(int fd, uint32_t eventMask, std::function<void()> cb)
    {
        return mock.registerCallback(fd, eventMask, std::move(cb));
    }
};

// 为我们的适配器提供 tag_invoke 实现
inline static auto tag_invoke(bind_to_t, Channel& ch, MockPollerAdapter& poller)
    -> std::expected<void, std::system_error>
{
    // 重用现有的实现逻辑来测试它
    if (auto res = poller.addFd(ch.fd()); !res)
        return res;
    if (ch.readableHandler()) {
        if (auto res = poller.registerCallback(ch.fd(), EPOLLIN, ch.readableHandler()); !res)
            return res;
    }
    if (ch.writableHandler()) {
        if (auto res = poller.registerCallback(ch.fd(), EPOLLOUT, ch.writableHandler()); !res)
            return res;
    }
    return {};
}

// --- 测试套件 ---
class BindToTest : public ::testing::Test {
protected:
    testing::StrictMock<MockPoller> mockPoller;
    MockPollerAdapter pollerAdapter { mockPoller };
};

// 测试：当 Channel 只有可读回调时
TEST_F(BindToTest, BindsChannelWithOnlyReadableHandler)
{
    using ::testing::_;
    using ::testing::Return;

    Channel ch(10); // 使用一个虚拟的文件描述符 10
    ch.setReadableHandler([] { });

    // 设置期望：
    // 1. addFd 必须被调用一次
    EXPECT_CALL(mockPoller, addFd(10)).WillOnce(Return(std::expected<void, std::system_error>()));
    // 2. registerCallback 必须被调用一次，且事件掩码为 EPOLLIN
    EXPECT_CALL(mockPoller, registerCallback(10, EPOLLIN, _)).WillOnce(Return(std::expected<void, std::system_error>()));

    // 执行绑定操作
    auto result = bind_to(ch, pollerAdapter);
    ASSERT_TRUE(result.has_value());
}

// 测试：当 Channel 有读和写两个回调时
TEST_F(BindToTest, BindsChannelWithBothHandlers)
{
    using ::testing::_;
    using ::testing::InSequence;
    using ::testing::Return;

    Channel ch(20);
    ch.setReadableHandler([] { });
    ch.setWritableHandler([] { });

    {
        InSequence seq; // 确保调用顺序正确

        EXPECT_CALL(mockPoller, addFd(20)).WillOnce(Return(std::expected<void, std::system_error>()));
        EXPECT_CALL(mockPoller, registerCallback(20, EPOLLIN, _)).WillOnce(Return(std::expected<void, std::system_error>()));
        EXPECT_CALL(mockPoller, registerCallback(20, EPOLLOUT, _)).WillOnce(Return(std::expected<void, std::system_error>()));
    }

    auto result = bind_to(ch, pollerAdapter);
    ASSERT_TRUE(result.has_value());
}

// 测试：当 Channel 没有任何回调时
TEST_F(BindToTest, BindsChannelWithNoHandlers)
{
    using ::testing::Return;

    Channel ch(30);

    EXPECT_CALL(mockPoller, addFd(30)).WillOnce(Return(std::expected<void, std::system_error>()));
    // 期望 registerCallback 完全不被调用
    EXPECT_CALL(mockPoller, registerCallback(testing::_, testing::_, testing::_)).Times(0);

    auto result = bind_to(ch, pollerAdapter);
    ASSERT_TRUE(result.has_value());
}

// 测试：当 addFd 失败时，绑定应该失败并且不注册任何回调
TEST_F(BindToTest, FailsIfAddFdFails)
{
    using ::testing::_;
    using ::testing::Return;

    Channel ch(40); // fd 是 40
    ch.setReadableHandler([] { });

    // 模拟 addFd 失败
    std::system_error error(EIO, std::system_category(), "Simulated I/O Error");

    // **修正点**: 确保 EXPECT_CALL 中的 fd 与上面的 Channel(40) 匹配
    EXPECT_CALL(mockPoller, addFd(40))
        .WillOnce(Return(std::unexpected(error)));

    // 期望 registerCallback 完全不被调用
    EXPECT_CALL(mockPoller, registerCallback(_, _, _)).Times(0);

    auto result = bind_to(ch, pollerAdapter);

    // 断言结果
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code(), error.code());
}