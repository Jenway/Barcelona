#include "HttpProtocolHandler.hpp"
#include "HttpStatus.hpp"
#include "IRequestHandler.hpp"
#include "IRequestParser.hpp"
#include "IResponseWriter.hpp"
#include "ISinker.hpp"
#include "Message.hpp"
#include "Status.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>
#include <string_view>

using namespace ::testing;

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

class MockRequestParser : public http::IRequestParser {
public:
    MOCK_METHOD((std::expected<IRequestParser::State, http::StatusCode>), parse, (std::string_view data), (override));
    MOCK_METHOD(const http::Request&, getRequest, (), (const, override));
    MOCK_METHOD(void, reset, (), (override));
};

class MockRequestHandler : public http::IRequestHandler {
public:
    MOCK_METHOD((std::expected<http::Response, std::error_code>), handleRequest, (const http::Request&), (override));
    MOCK_METHOD(http::Response, handleError, (http::StatusCode), (override));
};

class MockResponseWriter : public http::IResponseWriter {
public:
    MOCK_METHOD(void, bind_to, (http::Response), (override));
    MOCK_METHOD((std::expected<core::WriteResult, std::error_code>), writeTo, (ISinker & sinker), (override));
    MOCK_METHOD(bool, isKeepAlive, (), (const, override));
    MOCK_METHOD(void, reset, (), (override));
};

class HttpProtocolHandlerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        auto parser = std::make_unique<MockRequestParser>();
        mock_parser_ptr = parser.get();

        auto request_handler = std::make_unique<MockRequestHandler>();
        mock_request_handler_ptr = request_handler.get();

        auto response_writer = std::make_unique<NiceMock<MockResponseWriter>>();
        mock_response_writer_ptr = response_writer.get();

        // 创建被测试对象，注入所有 Mocks
        handler = std::make_unique<http::HttpProtocolHandler>(
            std::move(parser),
            std::move(response_writer),
            std::move(request_handler));

        // 设置默认行为
        ON_CALL(*mock_parser_ptr, getRequest()).WillByDefault(ReturnRef(request));
        ON_CALL(*mock_request_handler_ptr, handleRequest(_))
            .WillByDefault(Return(std::expected<http::Response, std::error_code>(controlled_response)));
    }

    std::unique_ptr<http::HttpProtocolHandler> handler;

    // 持有所有 Mocks 的指针
    MockSinker mock_sinker;
    MockRequestParser* mock_parser_ptr;
    MockRequestHandler* mock_request_handler_ptr;
    MockResponseWriter* mock_response_writer_ptr;

    http::Request request;
    http::Response controlled_response;
};

TEST_F(HttpProtocolHandlerTest, InitialStateWantsRead)
{
    EXPECT_EQ(handler->getStatus(), core::protocol::Status::WantRead);
}

TEST_F(HttpProtocolHandlerTest, InMemoryResponse)
{
    InSequence seq;

    // 1. Setup
    controlled_response.body = std::vector<char> { 'O', 'K' };
    controlled_response.headers = { { "Connection", "close" } };

    // 2. Expectations
    EXPECT_CALL(*mock_parser_ptr, parse(_))
        .WillOnce(Return(http::IRequestParser::State::Completed));

    EXPECT_CALL(*mock_parser_ptr, getRequest());

    EXPECT_CALL(*mock_request_handler_ptr, handleRequest(_))
        .WillOnce(Return(std::expected<http::Response, std::error_code>(controlled_response)));

    EXPECT_CALL(*mock_response_writer_ptr, bind_to(Field(&http::Response::body, VariantWith<std::vector<char>>(ElementsAre('O', 'K')))));
    EXPECT_CALL(*mock_response_writer_ptr, writeTo(Ref(mock_sinker)))
        .WillOnce(Return(core::WriteResult { .status = core::WriteResult::Status::Finished }));
    EXPECT_CALL(*mock_response_writer_ptr, isKeepAlive()).WillOnce(Return(false));

    // 3. Drive
    handler->onData("REQ");
    handler->onWriteReady(mock_sinker);

    // 4. Assert
    EXPECT_EQ(handler->getStatus(), core::protocol::Status::Finished);
}

TEST_F(HttpProtocolHandlerTest, FileBodyResponse)
{
    InSequence seq;

    // 1. Setup
    auto file_body = http::FileBody { .fd = 42, .size = 512, .offset = 0 };
    controlled_response.body = file_body;
    controlled_response.headers = { { "Connection", "close" } };

    // 2. Expectations
    EXPECT_CALL(*mock_parser_ptr, parse(_))
        .WillOnce(Return(http::IRequestParser::State::Completed));
    EXPECT_CALL(*mock_parser_ptr, getRequest());

    EXPECT_CALL(*mock_request_handler_ptr, handleRequest(_))
        .WillOnce(Return(std::expected<http::Response, std::error_code>(controlled_response)));

    EXPECT_CALL(*mock_response_writer_ptr, bind_to(Field(&http::Response::body, VariantWith<http::FileBody>(Field(&http::FileBody::fd, 42)))));
    EXPECT_CALL(*mock_response_writer_ptr, writeTo(Ref(mock_sinker)))
        .WillOnce(Return(core::WriteResult { .status = core::WriteResult::Status::Finished }));
    EXPECT_CALL(*mock_response_writer_ptr, isKeepAlive()).WillOnce(Return(false));

    // 3. Drive
    handler->onData("REQ");
    handler->onWriteReady(mock_sinker);

    // 4. Assert
    EXPECT_EQ(handler->getStatus(), core::protocol::Status::Finished);
}

TEST_F(HttpProtocolHandlerTest, PartialRequestParse)
{
    InSequence seq;

    // 1. Setup
    controlled_response.body = std::vector<char> {};
    controlled_response.headers = { { "Connection", "close" } };

    // 2. Expectations
    EXPECT_CALL(*mock_parser_ptr, parse(StrEq("PART1")))
        .WillOnce(Return(http::IRequestParser::State::Parsing));
    EXPECT_CALL(*mock_parser_ptr, parse(StrEq("PART2")))
        .WillOnce(Return(http::IRequestParser::State::Completed));
    EXPECT_CALL(*mock_request_handler_ptr, handleRequest(_));
    EXPECT_CALL(*mock_response_writer_ptr, bind_to(_));
    EXPECT_CALL(*mock_response_writer_ptr, writeTo(Ref(mock_sinker)))
        .WillOnce(Return(core::WriteResult { .status = core::WriteResult::Status::Finished, .bytes_sent = 100 }));
    EXPECT_CALL(*mock_response_writer_ptr, isKeepAlive()).WillOnce(Return(false));

    // 3. Drive
    handler->onData("PART1");
    handler->onData("PART2");
    handler->onWriteReady(mock_sinker);

    // 4. Assert
    EXPECT_EQ(handler->getStatus(), core::protocol::Status::Finished);
}

TEST_F(HttpProtocolHandlerTest, KeepAliveCycle)
{
    InSequence seq;

    // --- First Request (keep-alive) ---
    {
        EXPECT_CALL(*mock_parser_ptr, parse(StrEq("REQ1")))
            .WillOnce(Return(http::IRequestParser::State::Completed));
        EXPECT_CALL(*mock_request_handler_ptr, handleRequest(_));
        EXPECT_CALL(*mock_response_writer_ptr, bind_to(_));
        EXPECT_CALL(*mock_response_writer_ptr, writeTo(Ref(mock_sinker)))
            .WillOnce(Return(core::WriteResult { .status = core::WriteResult::Status::Finished, .bytes_sent = 1 }));
        EXPECT_CALL(*mock_response_writer_ptr, isKeepAlive()).WillOnce(Return(true));
        EXPECT_CALL(*mock_parser_ptr, reset());
        EXPECT_CALL(*mock_response_writer_ptr, reset());

        handler->onData("REQ1");
        handler->onWriteReady(mock_sinker);
        ASSERT_EQ(handler->getStatus(), core::protocol::Status::WantRead);
    }

    // --- Second Request (close) ---
    {
        EXPECT_CALL(*mock_parser_ptr, parse(StrEq("REQ2")))
            .WillOnce(Return(http::IRequestParser::State::Completed));
        EXPECT_CALL(*mock_request_handler_ptr, handleRequest(_));
        EXPECT_CALL(*mock_response_writer_ptr, bind_to(_));
        EXPECT_CALL(*mock_response_writer_ptr, writeTo(Ref(mock_sinker)))
            .WillOnce(Return(core::WriteResult { .status = core::WriteResult::Status::Finished, .bytes_sent = 1 }));
        EXPECT_CALL(*mock_response_writer_ptr, isKeepAlive()).WillOnce(Return(false));

        handler->onData("REQ2");
        handler->onWriteReady(mock_sinker);
        EXPECT_EQ(handler->getStatus(), core::protocol::Status::Finished);
    }
}