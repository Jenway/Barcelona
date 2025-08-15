// in tests/http_test/exact_router_test.cc
#include "http/common/HttpStatus.hpp"
#include "http/common/Message.hpp"
#include "http/routing/ExactRouter.hpp"
#include "http/utils/ResponseFactory.hpp"
#include "gtest/gtest.h"
#include <memory>

using namespace ::testing;

// Test Fixture
class ExactRouterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        router = std::make_unique<http::ExactRouter>();
    }

    std::unique_ptr<http::ExactRouter> router;
    http::Request request;
};

// 测试 1: 没有任何处理器注册时，应返回 404
TEST_F(ExactRouterTest, NoHandlersReturnsNotFound)
{
    request.uri = "/any/path";
    auto response_or_error = router->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 404);
}

// 测试 2: 方法和路径都匹配时，处理器被调用
TEST_F(ExactRouterTest, CorrectMethodAndPathIsMatched)
{
    bool was_called = false;
    http::ConcreteHandler handler = [&](const http::Request&) {
        was_called = true;
        return http::responses::createStockResponse<http::StatusCode::Ok>();
    };
    router->addHandler(http::Method::POST, "/login", handler);

    request.method = http::Method::POST;
    request.uri = "/login";
    auto response_or_error = router->handleRequest(request);

    EXPECT_TRUE(was_called);
    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 200);
}

// 测试 3: 路径匹配但方法不匹配，应返回 404
TEST_F(ExactRouterTest, PathMatchButMethodMismatchReturnsNotFound)
{
    bool was_called = false;
    router->addHandler(http::Method::POST, "/login", [&](const auto&) {
        was_called = true;
        return http::responses::createStockResponse<http::StatusCode::Ok>();
    });

    request.method = http::Method::GET; // <--- 方法不匹配
    request.uri = "/login";
    auto response_or_error = router->handleRequest(request);

    EXPECT_FALSE(was_called);
    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 404);
}

// 测试 4: 方法匹配但路径不匹配，应返回 404
TEST_F(ExactRouterTest, MethodMatchButPathMismatchReturnsNotFound)
{
    bool was_called = false;
    router->addHandler(http::Method::GET, "/api/user", [&](const auto&) {
        was_called = true;
        return http::responses::createStockResponse<http::StatusCode::Ok>();
    });

    request.method = http::Method::GET;
    request.uri = "/api/user/123"; // <--- 路径不完全匹配
    auto response_or_error = router->handleRequest(request);

    EXPECT_FALSE(was_called);
    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 404);
}