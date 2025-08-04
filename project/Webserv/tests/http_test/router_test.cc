#include "IRequestHandler.hpp"
#include "Message.hpp"
#include "RequestRouter.hpp" // 被测试的类
#include "ResponseFactory.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>

using namespace ::testing;

class MockSubHandler : public http::IRequestHandler {
public:
    MOCK_METHOD((std::expected<http::Response, std::error_code>), handleRequest, (const http::Request&), (override));
    MOCK_METHOD(http::Response, handleError, (http::StatusCode), (override));
};

// Test Fixture
class RequestRouterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        router = std::make_unique<http::RequestRouter>();
    }

    std::unique_ptr<http::RequestRouter> router;
    http::Request request;
};

// 测试 1: 空路由表，任何请求都应该 404
TEST_F(RequestRouterTest, NoRoutesRegisteredReturnsNotFound)
{
    request.uri = "/any/path";
    auto response_or_error = router->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 404);
}

// 测试 2: 精确路由能被正确匹配和调用
TEST_F(RequestRouterTest, ExactRouteMatch)
{
    bool handler_called = false;
    http::ConcreteHandler handler = [&](const http::Request&) {
        handler_called = true;
        return http::responses::createStockResponse<http::StatusCode::Ok>();
    };
    router->addRoute(http::Method::GET, "/api/health", handler);

    request.method = http::Method::GET;
    request.uri = "/api/health";
    auto response_or_error = router->handleRequest(request);

    EXPECT_TRUE(handler_called);
    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 200);
}

// 测试 3: 方法不匹配，即使路径匹配，精确路由也不应被调用
TEST_F(RequestRouterTest, ExactRouteMethodMismatchReturnsNotFound)
{
    bool handler_called = false;
    router->addRoute(http::Method::GET, "/api/data", [&](const auto&) {
        handler_called = true;
        return http::responses::createStockResponse<http::StatusCode::Ok>();
    });

    request.method = http::Method::POST; // <--- 方法不匹配
    request.uri = "/api/data";
    auto response_or_error = router->handleRequest(request);

    EXPECT_FALSE(handler_called); // 确保处理器未被调用
    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 404);
}

// 测试 4: (关键!) 验证最长前缀匹配原则
TEST_F(RequestRouterTest, PrefixRouteUsesLongestMatch)
{
    // 准备两个 mock handler
    auto mock_root_handler = std::make_unique<StrictMock<MockSubHandler>>();
    auto mock_api_handler = std::make_unique<StrictMock<MockSubHandler>>();

    // 我们期望 /api/v1/users 这个请求被 /api/ 处理器捕获
    EXPECT_CALL(*mock_api_handler, handleRequest(_)).WillOnce(Return(http::responses::createStockResponse<http::StatusCode::Ok>()));
    // 我们严正声明，根处理器不应该被调用！
    EXPECT_CALL(*mock_root_handler, handleRequest(_)).Times(0);

    // 注册路由，注意顺序是随机的
    router->addRoute("/api/", std::move(mock_api_handler));
    router->addRoute("/", std::move(mock_root_handler));

    request.uri = "/api/v1/users";
    auto response_or_error = router->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 200);
}

// 测试 5: 当没有更具体的前缀时，应回退到根处理器
TEST_F(RequestRouterTest, PrefixRouteFallsBackToRoot)
{
    auto mock_root_handler = std::make_unique<StrictMock<MockSubHandler>>();
    auto mock_api_handler = std::make_unique<StrictMock<MockSubHandler>>();

    EXPECT_CALL(*mock_root_handler, handleRequest(_)).WillOnce(Return(http::responses::createStockResponse<http::StatusCode::Ok>()));
    EXPECT_CALL(*mock_api_handler, handleRequest(_)).Times(0);

    router->addRoute("/api/", std::move(mock_api_handler));
    router->addRoute("/", std::move(mock_root_handler));

    request.uri = "/index.html"; // 这个路径不匹配 /api/，但匹配 /
    auto response_or_error = router->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 200);
}

// 测试 6: (关键!) 验证精确路由的优先级高于前缀路由
TEST_F(RequestRouterTest, ExactRouteHasPrecedenceOverPrefix)
{
    auto mock_prefix_handler = std::make_unique<StrictMock<MockSubHandler>>();

    // 尽管 /login 匹配前缀 /，但我们期望精确路由被调用
    EXPECT_CALL(*mock_prefix_handler, handleRequest(_)).Times(0);

    bool exact_handler_called = false;
    router->addRoute(http::Method::POST, "/login", [&](const auto&) {
        exact_handler_called = true;
        return http::responses::createStockResponse<http::StatusCode::Accepted>();
    });
    router->addRoute("/", std::move(mock_prefix_handler));

    request.method = http::Method::POST;
    request.uri = "/login";
    auto response_or_error = router->handleRequest(request);

    EXPECT_TRUE(exact_handler_called);
    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 202);
}

// 测试 7: 验证 handleError 方法能正确委托
TEST_F(RequestRouterTest, HandleErrorDelegatesToFactory)
{
    // 请求解析器说“方法不认识”
    auto response1 = router->handleError(http::StatusCode::NotImplemented);
    EXPECT_EQ(response1.status_code, 501);
    EXPECT_EQ(response1.reason_phrase, "Not Implemented");

    // 请求解析器说“版本不支持”
    auto response2 = router->handleError(http::StatusCode::HTTPVersionNotSupported);
    EXPECT_EQ(response2.status_code, 505);
    EXPECT_EQ(response2.reason_phrase, "HTTP Version Not Supported");
}