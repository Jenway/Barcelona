#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "Poller.hpp"

class PollerTest : public ::testing::Test {
protected:
    Poller poller;
    int pipe_fds[2];

    void SetUp() override
    {
        ASSERT_EQ(pipe(pipe_fds), 0);
    }

    void TearDown() override
    {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
    }
};

TEST_F(PollerTest, AddAndRemoveFd)
{
    auto addResult = poller.addFd(pipe_fds[0]);
    ASSERT_TRUE(addResult.has_value());

    auto removeResult = poller.removeFd(pipe_fds[0]);
    ASSERT_TRUE(removeResult.has_value());
}

TEST_F(PollerTest, RemoveNonExistentFdFails)
{
    // pipe_fds[1] 是一个有效的文件描述符，但我们没有将它添加到 poller 中
    auto removeResult = poller.removeFd(pipe_fds[1]);
    ASSERT_FALSE(removeResult.has_value());
    // 现在 epoll_ctl(DEL) 应该会返回 ENOENT，因为 fd 有效但未注册
    EXPECT_EQ(removeResult.error().code(), std::errc::no_such_file_or_directory);
}

// 测试：当没有事件发生时，pollOnce 会超时
TEST_F(PollerTest, PollOnceTimesOutWhenNoEvents)
{
    std::atomic<bool> callback_triggered = false;

    ASSERT_TRUE(poller.addFd(pipe_fds[0]).has_value());
    ASSERT_TRUE(poller.registerCallback(pipe_fds[0], EPOLLIN, [&]() {
                          callback_triggered = true;
                      })
            .has_value());

    // 使用 10ms 的超时来调用 pollOnce
    // 由于管道中没有数据，不应该有任何事件
    auto pollResult = poller.pollOnce(10);

    ASSERT_TRUE(pollResult.has_value());
    // 确认回调函数没有被调用
    ASSERT_FALSE(callback_triggered);
}

// 测试：当事件发生时，正确的回调函数被调用
TEST_F(PollerTest, CallbackIsTriggeredOnEvent)
{
    std::atomic<int> callback_count = 0;

    ASSERT_TRUE(poller.addFd(pipe_fds[0]).has_value());
    // 为管道的读端注册一个可读事件（EPOLLIN）的回调
    ASSERT_TRUE(poller.registerCallback(pipe_fds[0], EPOLLIN, [&]() {
                          callback_count++;
                      })
            .has_value());

    // 向管道的写端写入一个字节，这将触发读端的 EPOLLIN 事件
    char buffer = 'x';
    ASSERT_EQ(write(pipe_fds[1], &buffer, 1), 1);

    // 等待事件，超时设为0，因为事件应该已经准备好了
    auto pollResult = poller.pollOnce(0);
    ASSERT_TRUE(pollResult.has_value());

    // 确认回调函数被调用了一次
    ASSERT_EQ(callback_count, 1);

    // 再次调用 poll，此时管道数据还未读取，epoll 仍然会报告可读
    pollResult = poller.pollOnce(0);
    ASSERT_TRUE(pollResult.has_value());
    // 回调应该再次被触发
    ASSERT_EQ(callback_count, 2);
}

// 测试：Poller 能否正确处理多个文件描述符
TEST_F(PollerTest, HandlesMultipleFds)
{
    // 使用 eventfd 作为第二个事件源，它比管道更适合用于简单的信号通知
    int efd = eventfd(0, EFD_NONBLOCK);
    ASSERT_NE(efd, -1);

    std::atomic<bool> pipe_callback_fired = false;
    std::atomic<bool> eventfd_callback_fired = false;

    // 添加并注册管道的回调
    ASSERT_TRUE(poller.addFd(pipe_fds[0]).has_value());
    ASSERT_TRUE(poller.registerCallback(pipe_fds[0], EPOLLIN, [&]() {
                          pipe_callback_fired = true;
                      })
            .has_value());

    // 添加并注册 eventfd 的回调
    ASSERT_TRUE(poller.addFd(efd).has_value());
    ASSERT_TRUE(poller.registerCallback(efd, EPOLLIN, [&]() {
                          eventfd_callback_fired = true;
                      })
            .has_value());

    // 触发 eventfd 事件
    uint64_t val = 1;
    ASSERT_EQ(write(efd, &val, sizeof(val)), sizeof(val));

    // 轮询一次，只应触发 eventfd 的回调
    ASSERT_TRUE(poller.pollOnce(0).has_value());
    ASSERT_FALSE(pipe_callback_fired);
    ASSERT_TRUE(eventfd_callback_fired);

    // 重置标志位并触发管道事件
    eventfd_callback_fired = false;
    char buffer = 'x';
    ASSERT_EQ(write(pipe_fds[1], &buffer, 1), 1);

    // 轮询一次，这次只应触发管道的回调
    // 注意：eventfd 在被读取之前会一直保持信号状态
    // 我们在此测试中忽略它，只关注新触发的管道事件
    ASSERT_TRUE(poller.pollOnce(0).has_value());
    ASSERT_TRUE(pipe_callback_fired);

    close(efd);
}

// 测试：在一个回调函数内部移除其自身的文件描述符
TEST_F(PollerTest, RemoveFdFromWithinCallback)
{
    std::atomic<int> callback_count = 0;
    int efd = eventfd(0, EFD_NONBLOCK);
    ASSERT_NE(efd, -1);

    ASSERT_TRUE(poller.addFd(efd).has_value());

    // 注册一个回调，它会调用 poller.removeFd() 来移除自己
    ASSERT_TRUE(poller.registerCallback(efd, EPOLLIN, [&]() {
                          callback_count++;
                          // 在回调内部移除自己
                          ASSERT_TRUE(poller.removeFd(efd).has_value());
                      })
            .has_value());

    // 第一次触发事件
    uint64_t val = 1;
    ASSERT_EQ(write(efd, &val, sizeof(val)), sizeof(val));

    // 第一次轮询，回调应被触发，fd 被移除
    ASSERT_TRUE(poller.pollOnce(0).has_value());
    ASSERT_EQ(callback_count, 1);

    // 第二次触发事件（即使fd已关闭，写入会失败但我们仍可尝试）
    write(efd, &val, sizeof(val));

    // 第二次轮询，因为 fd 已被移除，回调不应再被触发
    ASSERT_TRUE(poller.pollOnce(0).has_value());
    ASSERT_EQ(callback_count, 1); // 确认回调计数没有增加

    close(efd);
}