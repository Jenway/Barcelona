#include "http/common/HttpStatus.hpp"
#include "http/common/Message.hpp"
#include "http/handlers/StaticFileHandler.hpp" // 被测试的类
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>
#include <unistd.h> // for close()

using namespace ::testing;

// Test Fixture: 负责创建和销毁临时的文件系统环境
class StaticFileHandlerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        test_root = std::filesystem::temp_directory_path() / "static_handler_test_root";
        std::filesystem::create_directories(test_root / "css");

        const std::string index_content = "<html><body>Hello</body></html>";
        std::ofstream(test_root / "index.html") << index_content;
        index_html_size = index_content.size();

        const std::string css_content = "body { color: blue; }";
        std::ofstream(test_root / "css/style.css") << css_content;

        std::ofstream(test_root / "forbidden.txt") << "secret";
        std::filesystem::permissions(
            test_root / "forbidden.txt",
            std::filesystem::perms::none,
            std::filesystem::perm_options::replace);

        handler = std::make_unique<http::StaticFileHandler>(test_root, "/");
    }

    void TearDown() override
    {
        std::filesystem::remove_all(test_root);
    }

    // 辅助函数，确保文件描述符被关闭，防止测试泄漏资源
    void ensureFdClosed(http::Response& response)
    {
        if (auto* body = std::get_if<http::FileBody>(&response.body)) {
            if (body->fd != -1) {
                ::close(body->fd);
            }
        }
    }

    std::filesystem::path test_root;
    std::unique_ptr<http::StaticFileHandler> handler;
    http::Request request;
    size_t index_html_size {};
};

// --- GET 请求测试 ---

TEST_F(StaticFileHandlerTest, Get_HandlesBasicFileRequest)
{
    request.method = http::Method::GET;
    request.uri = "/index.html";
    auto response = handler->handleRequest(request); // <-- 调用统一入口

    EXPECT_EQ(response.status_code, 200);
    EXPECT_EQ(response.headers["Content-Type"], "text/html; charset=utf-8");
    EXPECT_EQ(response.headers["Content-Length"], std::to_string(index_html_size));
    ASSERT_TRUE(std::holds_alternative<http::FileBody>(response.body));

    ensureFdClosed(response);
}

TEST_F(StaticFileHandlerTest, Get_HandlesRequestToRoot)
{
    request.method = http::Method::GET;
    request.uri = "/";
    auto response = handler->handleRequest(request);

    EXPECT_EQ(response.status_code, 200);
    EXPECT_EQ(response.headers["Content-Length"], std::to_string(index_html_size));

    ensureFdClosed(response);
}

TEST_F(StaticFileHandlerTest, Get_ReturnsNotFoundForNonExistentFile)
{
    request.method = http::Method::GET;
    request.uri = "/non-existent-file.jpg";
    auto response = handler->handleRequest(request);
    EXPECT_EQ(response.status_code, 404);
}

TEST_F(StaticFileHandlerTest, Get_ReturnsForbiddenForDirectoryRequest)
{
    request.method = http::Method::GET;
    request.uri = "/css/";
    auto response = handler->handleRequest(request);
    EXPECT_EQ(response.status_code, 403);
}

TEST_F(StaticFileHandlerTest, Get_ReturnsForbiddenForUnreadableFile)
{
    request.method = http::Method::GET;
    request.uri = "/forbidden.txt";
    auto response = handler->handleRequest(request);
    EXPECT_EQ(response.status_code, 403);
}

TEST_F(StaticFileHandlerTest, Get_ReturnsBadRequestForPathTraversalAttempt)
{
    request.method = http::Method::GET;
    request.uri = "/../../../../etc/passwd";
    auto response = handler->handleRequest(request);
    EXPECT_EQ(response.status_code, 400);
}

// --- HEAD 请求测试 ---

TEST_F(StaticFileHandlerTest, Head_ReturnsHeadersOnly)
{
    request.method = http::Method::HEAD;
    request.uri = "/index.html";
    auto response = handler->handleRequest(request);

    EXPECT_EQ(response.status_code, 200);
    // 头部信息应该和 GET 完全一样
    EXPECT_EQ(response.headers["Content-Length"], std::to_string(index_html_size));
    EXPECT_EQ(response.headers["Content-Type"], "text/html; charset=utf-8");

    // 关键：验证 Body 是空的 vector<char> (默认 variant 状态)
    ASSERT_TRUE(std::holds_alternative<std::vector<char>>(response.body));
    EXPECT_TRUE(std::get<std::vector<char>>(response.body).empty());
}

TEST_F(StaticFileHandlerTest, Head_ReturnsNotFoundForNonExistentFile)
{
    request.method = http::Method::HEAD;
    request.uri = "/non-existent-file.jpg";
    auto response = handler->handleRequest(request);
    EXPECT_EQ(response.status_code, 404);
}

// --- DELETE 请求测试 ---

TEST_F(StaticFileHandlerTest, Delete_SuccessfullyDeletesFile)
{
    const auto file_to_delete = test_root / "css/style.css";
    ASSERT_TRUE(std::filesystem::exists(file_to_delete));

    request.method = http::Method::DELETE;
    request.uri = "/css/style.css";
    auto response = handler->handleRequest(request);

    EXPECT_EQ(response.status_code, 204); // <-- 验证 204 No Content
    EXPECT_TRUE(response.body.valueless_by_exception() || std::get<std::vector<char>>(response.body).empty()); // Body 应该为空

    EXPECT_FALSE(std::filesystem::exists(file_to_delete)); // <-- 验证文件真的被删了
}

TEST_F(StaticFileHandlerTest, DeleteReturnsNotFoundForNonExistentFile)
{
    request.method = http::Method::DELETE;
    request.uri = "/non-existent-file.css";
    auto response = handler->handleRequest(request);
    EXPECT_EQ(response.status_code, 404);
}

TEST_F(StaticFileHandlerTest, DeleteReturnsForbiddenForDirectory)
{
    request.method = http::Method::DELETE;
    request.uri = "/css/"; // 不允许删除目录
    auto response = handler->handleRequest(request);
    EXPECT_EQ(response.status_code, 403);
}

// --- 其他方法测试 ---

TEST_F(StaticFileHandlerTest, ReturnsMethodNotAllowedForPostRequest)
{
    request.method = http::Method::POST;
    request.uri = "/index.html";
    auto response = handler->handleRequest(request);
    EXPECT_EQ(response.status_code, 405);
}