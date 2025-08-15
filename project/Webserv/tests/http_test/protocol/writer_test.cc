#include "ISinker.hpp" // 它的依赖
#include "http/common/Message.hpp"
#include "http/core/ResponseWriter.hpp" // 被测试的类
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>
#include <string>

using namespace ::testing;

// 我们只需要一个 MockSinker
class MockSinker : public ISinker {
public:
    MOCK_METHOD((std::expected<core::WriteResult, std::error_code>),
        write,
        (const char* data, size_t len),
        (override));
    MOCK_METHOD((std::expected<core::WriteResult, std::error_code>),
        sendfile,
        (int in_fd, off_t& offset, size_t count),
        (override));
};

// 一个专门为 ResponseWriter 设计的 Test Fixture
class ResponseWriterTest : public ::testing::Test {
protected:
    void SetUp() override { }

    http::ResponseWriter writer;
    MockSinker mock_sinker;
    http::Response response;
};

// 测试1：最经典的情况 - 写入带有内存 body 的响应
TEST_F(ResponseWriterTest, WritesHeaderAndInMemoryBody)
{
    InSequence seq;

    // 1. Setup
    response.version = "HTTP/1.1";
    response.status_code = 200;
    response.reason_phrase = "OK";
    response.body = std::vector<char> { 'D', 'A', 'T', 'A' };
    writer.bind_to(std::move(response));

    // 2. Expectations - 我们精确地验证 write 被调用的顺序和内容
    // 第一次调用：写入头
    EXPECT_CALL(mock_sinker, write(StartsWith("HTTP/1.1 200 OK"), _))
        .WillOnce(Return(core::WriteResult { .status = core::WriteResult::Status::Finished }));

    // 第二次调用：写入 body
    EXPECT_CALL(mock_sinker, write(StrEq("DATA"), 4))
        .WillOnce(Return(core::WriteResult { .status = core::WriteResult::Status::Finished }));

    // 3. Drive and Assert
    auto result = writer.writeTo(mock_sinker);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, core::WriteResult::Status::Finished);
}

// 测试2：验证它能正确调用 sendfile
TEST_F(ResponseWriterTest, WritesHeaderAndFileBody)
{
    InSequence seq;

    // 1. Setup
    response.version = "HTTP/1.1";
    response.status_code = 200;
    response.reason_phrase = "OK";
    response.body = http::FileBody { .fd = 42, .size = 1024 };
    writer.bind_to(std::move(response));

    // 2. Expectations
    // 第一次调用：写入头
    EXPECT_CALL(mock_sinker, write(StartsWith("HTTP/1.1 200 OK"), _))
        .WillOnce(Return(core::WriteResult { .status = core::WriteResult::Status::Finished }));

    // 第二次调用：必须是 sendfile！
    EXPECT_CALL(mock_sinker, sendfile(42, _, 1024))
        .WillOnce(Return(core::WriteResult { .status = core::WriteResult::Status::Finished }));

    // 3. Drive and Assert
    auto result = writer.writeTo(mock_sinker);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, core::WriteResult::Status::Finished);
}

// 测试3：处理写入被阻塞的情况
TEST_F(ResponseWriterTest, HandlesPartialHeaderWrite)
{
    // 1. Setup
    response.body = std::vector<char> { 'D', 'A', 'T', 'A' };
    writer.bind_to(std::move(response));

    // 2. Expectations
    // 我们只期望一次对头的写入，并且这次写入是部分完成的 (Blocked)
    EXPECT_CALL(mock_sinker, write(_, _))
        .WillOnce(Return(core::WriteResult { .status = core::WriteResult::Status::Continue, .bytes_sent = 10 }));

    // **我们不期望对 body 的写入发生**

    // 3. Drive and Assert
    auto result = writer.writeTo(mock_sinker);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, core::WriteResult::Status::Continue); // 验证返回状态正确
    EXPECT_EQ(result->bytes_sent, 10);
}

// 测试4：处理 keep-alive 的逻辑
TEST_F(ResponseWriterTest, IsKeepAliveReturnsCorrectValue)
{
    // Case 1: Header is "keep-alive"
    response.headers["Connection"] = "keep-alive";
    writer.bind_to(response);
    EXPECT_TRUE(writer.isKeepAlive());

    // Case 2: Header is "close"
    response.headers["Connection"] = "close";
    writer.bind_to(response);
    EXPECT_FALSE(writer.isKeepAlive());

    // Case 3: No connection header (defaults to false)
    response.headers.clear();
    writer.bind_to(response);
    EXPECT_FALSE(writer.isKeepAlive());
}