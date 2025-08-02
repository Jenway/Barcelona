#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <system_error>

#include "Connection.hpp"
#include "IProtocolHandler.hpp"
#include "ISinker.hpp"
#include "ISource.hpp"
#include "Socket.hpp"
#include "Status.hpp"

using namespace ::testing;

// --- 1. 创建我们的 Mock/Dummy 对象 ---

// Mock ISource，让我们可以完全控制读操作的结果
class MockSource : public ISource {
public:
    MOCK_METHOD((std::expected<core::ReadResult, std::error_code>), read,
        (std::vector<char> & buffer), (override));
};

// Mock ISinker，让我们可以控制写操作的结果并验证它是否被调用
class MockSinker : public ISinker {
public:
    MOCK_METHOD((std::expected<core::WriteResult, std::error_code>), write,
        (const char* data, size_t len), (override));
    MOCK_METHOD((std::expected<core::WriteResult, std::error_code>), sendfile,
        (int in_fd, off_t& offset, size_t count), (override));
};

// Mock IHandler，完全控制协议的行为
class MockHandler : public protocol::IHandler {
public:
    MOCK_METHOD(void, onData, (std::string_view data), (override));
    MOCK_METHOD(void, onReadEOF, (), (override));

    MOCK_METHOD((std::expected<core::WriteResult, std::error_code>), onWriteReady,
        (ISinker & sinker), (override));
    MOCK_METHOD(core::protocol::Status, getStatus, (), (const, override));
};

// --- 2. 编写测试套件 ---

class ConnectionTest : public Test {
protected:
    // 我们用一个无效的 fd(-1) 来创建 Socket，因为测试中不会真的去读写它
    // 真正的 I/O 操作由 MockSource 和 MockSinker 模拟
    Socket socket_;
    std::unique_ptr<MockHandler> handler_ = std::make_unique<MockHandler>();
    std::unique_ptr<MockSource> source_ = std::make_unique<MockSource>();
    std::unique_ptr<MockSinker> sinker_ = std::make_unique<MockSinker>();

    // GTest 需要知道裸指针，以便设置期望
    MockHandler* handler_ptr_ = handler_.get();
    MockSource* source_ptr_ = source_.get();
    MockSinker* sinker_ptr_ = sinker_.get();

    // 被测试的对象
    Connection connection_ { std::move(socket_), std::move(handler_), std::move(source_),
        std::move(sinker_) };
};

// 测试 1: 一个完整的“读 -> 写 -> 完成”的开心路径
TEST_F(ConnectionTest, FullCycleHappyPath)
{
    // --- 阶段 1: 读 ---
    EXPECT_CALL(*source_ptr_, read(_))
        .WillOnce(Return(core::ReadResult { .status = core::ReadResult::Status::GotData, .bytes_read = 4 }));
    EXPECT_CALL(*handler_ptr_, onData(_));
    EXPECT_CALL(*handler_ptr_, getStatus()).WillOnce(Return(core::protocol::Status::WantWrite));

    auto result = connection_.onReadable();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(connection_.interestedEvents(), POLL_OUT);

    // --- 阶段 2: 写 ---
    // 期望 onWritable 被调用时，handler_->onWriteReady() 会被调用
    // 并且我们模拟它一次性发送完成
    core::WriteResult finished_result { .status = core::WriteResult::Status::Finished, .bytes_sent = 100 };
    EXPECT_CALL(*handler_ptr_, onWriteReady(_)).WillOnce(Return(finished_result));

    // 期望在发送完成后，Connection 再次询问协议状态，此时协议应该说完成了
    EXPECT_CALL(*handler_ptr_, getStatus()).WillOnce(Return(core::protocol::Status::Finished));

    result = connection_.onWritable();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(connection_.isClosed());
}

// 测试 2: 测试 Sinker 需要多次写入的情况
TEST_F(ConnectionTest, PartialWrite)
{
    // 初始设置，让 Connection 进入 WRITING 状态
    EXPECT_CALL(*source_ptr_, read(_)).WillOnce(Return(core::ReadResult { .status = core::ReadResult::Status::GotData, .bytes_read = 4 }));
    EXPECT_CALL(*handler_ptr_, onData(_));
    EXPECT_CALL(*handler_ptr_, getStatus()).WillOnce(Return(core::protocol::Status::WantWrite));
    connection_.onReadable();
    ASSERT_EQ(connection_.interestedEvents(), POLL_OUT);

    // --- 第一次写 ---
    // 模拟 Handler 只写了一部分数据，需要继续写
    core::WriteResult continue_result { .status = core::WriteResult::Status::Continue, .bytes_sent = 50 };
    EXPECT_CALL(*handler_ptr_, onWriteReady(_)).WillOnce(Return(continue_result));

    auto result = connection_.onWritable();
    ASSERT_TRUE(result.has_value());
    // 验证状态：Connection 应该保持 WRITING 状态
    EXPECT_EQ(connection_.interestedEvents(), POLL_OUT);
    EXPECT_FALSE(connection_.isClosed());

    // --- 第二次写 ---
    // 再次触发 onWritable
    // 这次我们模拟 Handler 发送完成
    core::WriteResult finished_result { .status = core::WriteResult::Status::Finished, .bytes_sent = 50 };
    EXPECT_CALL(*handler_ptr_, onWriteReady(_)).WillOnce(Return(finished_result));
    EXPECT_CALL(*handler_ptr_, getStatus()).WillOnce(Return(core::protocol::Status::Finished));

    result = connection_.onWritable();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(connection_.isClosed());
}

// 测试 3: 测试 Source 返回 EOF
TEST_F(ConnectionTest, HandleEofOnRead)
{
    // 模拟 Source 返回 EOF
    EXPECT_CALL(*source_ptr_, read(_)).WillOnce(Return(core::ReadResult { .status = core::ReadResult::Status::Eof, .bytes_read = 0 }));
    // 期望 Connection 在收到 EOF 时，会通知 Handler
    EXPECT_CALL(*handler_ptr_, onReadEOF());

    auto result = connection_.onReadable();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(connection_.isClosed());
}

// 测试 4: 当协议处理器直接出错时
TEST_F(ConnectionTest, ProtocolHandlerReturnsError)
{
    // 阶段 1: 正常读取
    EXPECT_CALL(*source_ptr_, read(_))
        .WillOnce(Return(core::ReadResult { .status = core::ReadResult::Status::GotData, .bytes_read = 10 }));
    EXPECT_CALL(*handler_ptr_, onData(_));

    // 阶段 2: 处理完数据后，协议处理器报告内部错误
    EXPECT_CALL(*handler_ptr_, getStatus()).WillOnce(Return(core::protocol::Status::Error));

    // 触发读事件
    auto result = connection_.onReadable();
    ASSERT_TRUE(result.has_value());

    // 验证状态：Connection 应该立即关闭
    EXPECT_TRUE(connection_.isClosed());
}

// 测试 5: 当 Sinker 在写入时返回一个真正的 I/O 错误
TEST_F(ConnectionTest, SinkerReturnsIoError)
{
    // 设置进入 WRITING 状态
    EXPECT_CALL(*source_ptr_, read(_)).WillOnce(Return(core::ReadResult { .status = core::ReadResult::Status::GotData, .bytes_read = 4 }));
    EXPECT_CALL(*handler_ptr_, onData(_));
    EXPECT_CALL(*handler_ptr_, getStatus()).WillOnce(Return(core::protocol::Status::WantWrite));
    connection_.onReadable();
    ASSERT_EQ(connection_.interestedEvents(), POLL_OUT);

    // 模拟 Handler 在写入时返回一个 I/O 错误
    std::error_code broken_pipe = std::make_error_code(std::errc::broken_pipe);
    EXPECT_CALL(*handler_ptr_, onWriteReady(_)).WillOnce(Return(std::unexpected(broken_pipe)));

    auto result = connection_.onWritable();
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code(), broken_pipe);
    EXPECT_TRUE(connection_.isClosed());
}

// 测试 6: 读取时发生致命 I/O 错误
TEST_F(ConnectionTest, SourceReturnsIoError)
{
    std::error_code permission_denied = std::make_error_code(std::errc::permission_denied);
    EXPECT_CALL(*source_ptr_, read(_)).WillOnce(Return(std::unexpected(permission_denied)));

    auto result = connection_.onReadable();
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code(), permission_denied);
    EXPECT_TRUE(connection_.isClosed());
}

// 测试 6: 写入时发生致命 I/O 错误
TEST_F(ConnectionTest, HandlerReturnsIoErrorOnWrite)
{
    // 进入 WRITING 状态
    EXPECT_CALL(*source_ptr_, read(_)).WillOnce(Return(core::ReadResult { .status = core::ReadResult::Status::GotData, .bytes_read = 4 }));
    EXPECT_CALL(*handler_ptr_, onData(_));
    EXPECT_CALL(*handler_ptr_, getStatus()).WillOnce(Return(core::protocol::Status::WantWrite));
    connection_.onReadable();

    std::error_code broken_pipe = std::make_error_code(std::errc::broken_pipe);
    EXPECT_CALL(*handler_ptr_, onWriteReady(_)).WillOnce(Return(std::unexpected(broken_pipe)));

    auto result = connection_.onWritable();
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code(), broken_pipe);
    EXPECT_TRUE(connection_.isClosed());
}