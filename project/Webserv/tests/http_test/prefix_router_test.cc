// in tests/http_test/prefix_router_test.cc
#include "http/core/HttpStatus.hpp"
#include "http/core/Message.hpp"
#include "http/interfaces/IRequestHandler.hpp"
#include "http/routing/PrefixRouter.hpp"
#include "http/utils/ResponseFactory.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>

using namespace ::testing;

namespace {
class MockRequestHandler : public http::IRequestHandler {
public:
    MOCK_METHOD((std::expected<http::Response, std::error_code>), handleRequest, (const http::Request&), (override));
};

// Test Fixture
class PrefixRouterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        router = std::make_unique<http::PrefixRouter>();
    }

    std::unique_ptr<http::PrefixRouter> router;
    http::Request request;
};

} // namespace

// 测试 1: 没有任何处理器注册时，应返回 404
TEST_F(PrefixRouterTest, NoHandlersReturnsNotFound)
{
    request.uri = "/any/path";
    auto response_or_error = router->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 404);
}

// 测试 2: 只有一个处理器时，能正确匹配
TEST_F(PrefixRouterTest, SingleHandlerMatch)
{
    auto mock_handler = std::make_unique<StrictMock<MockRequestHandler>>();
    EXPECT_CALL(*mock_handler, handleRequest(_))
        .WillOnce(Return(http::responses::createStockResponse<http::StatusCode::Ok>()));

    router->addHandler("/api/", std::move(mock_handler));

    request.uri = "/api/users";
    auto response_or_error = router->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 200);
}

// 测试 3: (核心测试!) 验证最长前缀匹配原则
TEST_F(PrefixRouterTest, UsesLongestPrefixMatch)
{
    auto mock_root = std::make_unique<StrictMock<MockRequestHandler>>();
    auto mock_api = std::make_unique<StrictMock<MockRequestHandler>>();
    auto mock_api_v1 = std::make_unique<StrictMock<MockRequestHandler>>();

    // 期望：/api/v1/status 请求应该被 /api/v1/ 处理器捕获
    EXPECT_CALL(*mock_api_v1, handleRequest(_))
        .WillOnce(Return(http::responses::createStockResponse<http::StatusCode::Ok>()));
    // 严正声明：其他两个更短的前缀处理器不应该被调用
    EXPECT_CALL(*mock_api, handleRequest(_)).Times(0);
    EXPECT_CALL(*mock_root, handleRequest(_)).Times(0);

    // 以随机顺序注册处理器
    router->addHandler("/", std::move(mock_root));
    router->addHandler("/api/v1/", std::move(mock_api_v1));
    router->addHandler("/api/", std::move(mock_api));

    request.uri = "/api/v1/status";
    router->handleRequest(request);
}

// 测试 4: 当没有更具体的前缀时，应回退到更通用的前缀 (例如 "/")
TEST_F(PrefixRouterTest, FallsBackToShorterPrefix)
{
    auto mock_root = std::make_unique<StrictMock<MockRequestHandler>>();
    auto mock_api = std::make_unique<StrictMock<MockRequestHandler>>();

    // 期望：/assets/style.css 请求应该被 / 处理器捕获
    EXPECT_CALL(*mock_root, handleRequest(_))
        .WillOnce(Return(http::responses::createStockResponse<http::StatusCode::Ok>()));
    // 严正声明：/api/ 处理器不应该被调用
    EXPECT_CALL(*mock_api, handleRequest(_)).Times(0);

    router->addHandler("/api/", std::move(mock_api));
    router->addHandler("/", std::move(mock_root));

    request.uri = "/assets/style.css";
    router->handleRequest(request);
}

// 测试 5: 如果没有任何前缀能匹配，应返回 404
TEST_F(PrefixRouterTest, NoMatchingPrefixReturnsNotFound)
{
    auto mock_handler = std::make_unique<StrictMock<MockRequestHandler>>();
    EXPECT_CALL(*mock_handler, handleRequest(_)).Times(0);
    router->addHandler("/api/", std::move(mock_handler));

    request.uri = "/status/health"; // 这个 URI 不匹配任何已注册的前缀
    auto response_or_error = router->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 404);
}