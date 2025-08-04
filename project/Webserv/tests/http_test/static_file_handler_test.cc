#include "http/core/HttpStatus.hpp"
#include "http/core/Message.hpp"
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
        std::filesystem::create_directories(test_root / "subdir");

        // **1. 将文件内容定义为变量**
        const std::string index_content = "<html><body>Hello</body></html>";
        const std::string css_content = "body { color: blue; }";
        const std::string forbidden_content = "secret";

        // **2. 在写入文件的同时，将大小记录到成员变量中**
        std::ofstream(test_root / "index.html") << index_content;
        index_html_size = index_content.size();

        std::ofstream(test_root / "css/style.css") << css_content;
        std::ofstream(test_root / "forbidden.txt") << forbidden_content;

        std::filesystem::permissions(
            test_root / "forbidden.txt",
            std::filesystem::perms::none,
            std::filesystem::perm_options::replace);

        handler = std::make_unique<http::StaticFileHandler>(test_root);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(test_root);
    }

    void ensureFdClosed(http::Response& response)
    {
        if (auto* body = std::get_if<http::FileBody>(&response.body)) {
            if (body->fd != -1) {
                ::close(body->fd);
            }
        }
    }

    // --- Fixture 成员变量 ---
    std::filesystem::path test_root;
    std::unique_ptr<http::StaticFileHandler> handler;
    http::Request request;

    // **3. 新增成员变量来存储测试数据的信息**
    size_t index_html_size {};
};

// --- 开始不留情面的测试！ ---

TEST_F(StaticFileHandlerTest, HandlesBasicFileRequest)
{
    request.uri = "/index.html";
    auto response_or_error = handler->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    auto& response = *response_or_error;
    EXPECT_EQ(response.status_code, 200);
    EXPECT_EQ(response.headers["Content-Type"], "text/html; charset=utf-8");

    // **4. 使用成员变量进行断言，而不是硬编码的 "31"**
    EXPECT_EQ(response.headers["Content-Length"], std::to_string(index_html_size));

    ASSERT_TRUE(std::holds_alternative<http::FileBody>(response.body));

    ensureFdClosed(response);
}

TEST_F(StaticFileHandlerTest, HandlesRequestToRoot)
{
    request.uri = "/";
    auto response_or_error = handler->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    auto& response = *response_or_error;
    EXPECT_EQ(response.status_code, 200);

    // **5. 同样在这里使用成员变量**
    EXPECT_EQ(response.headers["Content-Length"], std::to_string(index_html_size));

    ensureFdClosed(response);
}

TEST_F(StaticFileHandlerTest, ReturnsNotFoundForNonExistentFile)
{
    request.uri = "/non-existent-file.jpg";
    auto response_or_error = handler->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 404);
}

TEST_F(StaticFileHandlerTest, ReturnsForbiddenForDirectoryRequest)
{
    request.uri = "/css/"; // 请求一个目录是禁止的
    auto response_or_error = handler->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 403);
}

TEST_F(StaticFileHandlerTest, ReturnsForbiddenForUnreadableFile)
{
    request.uri = "/forbidden.txt"; // 这个文件我们在 SetUp 中设为了不可读
    auto response_or_error = handler->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 403);
}

TEST_F(StaticFileHandlerTest, ReturnsBadRequestForPathTraversalAttempt)
{
    request.uri = "/../../../../etc/passwd"; // 经典的路径遍历攻击
    auto response_or_error = handler->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 400);
}

TEST_F(StaticFileHandlerTest, ReturnsMethodNotAllowedForPostRequest)
{
    request.method = http::Method::POST; // StaticFileHandler 只支持 GET
    request.uri = "/index.html";
    auto response_or_error = handler->handleRequest(request);

    ASSERT_TRUE(response_or_error.has_value());
    EXPECT_EQ(response_or_error->status_code, 405);
}