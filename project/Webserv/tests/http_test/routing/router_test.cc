// in tests/http_test/router_test.cc
#include "http/common/HttpStatus.hpp"
#include "http/common/Message.hpp"
#include "http/interfaces/IRequestHandler.hpp"
#include "http/routing/RequestRouter.hpp"
#include "http/utils/ResponseFactory.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>

using namespace ::testing;

namespace {
class MockRequestHandler : public http::IRequestHandler {
public:
    using ResponseResult = std::expected<http::Response, std::error_code>;

    // 构造和析构日志可以暂时保留，看看这次能不能打印出来
    MockRequestHandler()
    {
        fmt::print(">>>> MockRequestHandler CREATED at {}", (void*)this);
    }
    ~MockRequestHandler() override
    {
        fmt::print("<<<< MockRequestHandler DESTROYED at {}", (void*)this);
    }

    MOCK_METHOD(ResponseResult, handleRequest, (const http::Request&), (override));
};
} // namespace

// Test Fixture
class RequestRouterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        fmt::print("--- Test Case START: {} ---",
            ::testing::UnitTest::GetInstance()->current_test_info()->name());
        router = std::make_unique<http::RequestRouter>();
    }

    void TearDown() override
    {
        fmt::print("--- Test Case END: {} ---",
            ::testing::UnitTest::GetInstance()->current_test_info()->name());
    }

    std::unique_ptr<http::RequestRouter> router;
    http::Request request;
};

// --- 测试用例保持不变，但现在它们会被日志包围 ---

TEST_F(RequestRouterTest, EmptyPipelineReturnsNotFound)
{
    auto response_or_error = router->handleRequest(request);
    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 404);
}

TEST_F(RequestRouterTest, StopsAtFirstSuccessfulHandler)
{
    auto handler1 = std::make_unique<StrictMock<MockRequestHandler>>();
    auto handler2 = std::make_unique<StrictMock<MockRequestHandler>>();

    EXPECT_CALL(*handler1, handleRequest(_))
        .WillOnce(Return(http::responses::createStockResponse<http::StatusCode::Ok>()));
    EXPECT_CALL(*handler2, handleRequest(_)).Times(0);

    router->use(std::move(handler1));
    router->use(std::move(handler2));

    auto response_or_error = router->handleRequest(request);
    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 200);
}

TEST_F(RequestRouterTest, ContinuesToNextHandlerOnNotFound)
{
    auto handler1 = std::make_unique<StrictMock<MockRequestHandler>>();
    auto handler2 = std::make_unique<StrictMock<MockRequestHandler>>();

    EXPECT_CALL(*handler1, handleRequest(_))
        .WillOnce(Return(http::responses::createStockResponse<http::StatusCode::NotFound>()));
    EXPECT_CALL(*handler2, handleRequest(_))
        .WillOnce(Return(http::responses::createStockResponse<http::StatusCode::Accepted>()));

    router->use(std::move(handler1));
    router->use(std::move(handler2));

    auto response_or_error = router->handleRequest(request);
    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 202);
}

TEST_F(RequestRouterTest, ReturnsNotFoundIfAllHandlersFail)
{
    auto handler1 = std::make_unique<StrictMock<MockRequestHandler>>();
    auto handler2 = std::make_unique<StrictMock<MockRequestHandler>>();

    EXPECT_CALL(*handler1, handleRequest(_))
        .WillOnce(Return(http::responses::createStockResponse<http::StatusCode::NotFound>()));
    EXPECT_CALL(*handler2, handleRequest(_))
        .WillOnce(Return(http::responses::createStockResponse<http::StatusCode::NotFound>()));

    router->use(std::move(handler1));
    router->use(std::move(handler2));

    auto response_or_error = router->handleRequest(request);
    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 404);
}