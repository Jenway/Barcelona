#include "Status.hpp"
#include "http/core/RequestParser.hpp"
#include "logger.hpp"
#include "gtest/gtest.h"
#include <expected>
#include <vector>

using http::IRequestParser;
using State = IRequestParser::State;

class RequestParserTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        parser = std::make_unique<http::RequestParser>();
    }

    std::unique_ptr<http::RequestParser> parser;
};

TEST_F(RequestParserTest, HandlesSimpleGETRequest)
{
    const std::string request_str = "GET /test/index.html HTTP/1.0\r\n"
                                    "Host: www.example.com\r\n"
                                    "User-Agent: Test-Client\r\n\r\n";

    auto res = parser->parse(request_str);
    if (!res) {
        auto err_ = res.error();
        LOG_ERROR("Error: {}", err_);
    }
    // 1. 成功返回
    ASSERT_TRUE(res.has_value());
    // 2. 状态是 Completed
    EXPECT_EQ(res.value(), State::Completed);

    const auto& req = parser->getRequest();
    EXPECT_EQ(req.method, http::Method::GET);
    EXPECT_EQ(req.uri, "/test/index.html");
    EXPECT_EQ(req.version, "HTTP/1.0");
    EXPECT_EQ(req.headers.size(), 2u);
    EXPECT_EQ(req.headers.at("Host"), "www.example.com");
    EXPECT_EQ(req.headers.at("User-Agent"), "Test-Client");
    EXPECT_TRUE(req.body.empty());
}

TEST_F(RequestParserTest, HandlesPOSTRequestWithBody)
{
    const std::string expected_body = R"({"key":"value"})";
    const std::string request_str = "POST /api/data HTTP/1.0\r\n"
                                    "Content-Type: application/json\r\n"
                                    "Content-Length: 15\r\n\r\n"
        + expected_body;

    auto res = parser->parse(request_str);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), State::Completed);

    const auto& req = parser->getRequest();
    EXPECT_EQ(req.method, http::Method::POST);
    EXPECT_EQ(req.uri, "/api/data");
    EXPECT_EQ(req.version, "HTTP/1.0");
    EXPECT_EQ(req.headers.at("Content-Type"), "application/json");
    EXPECT_EQ(req.headers.at("Content-Length"), "15");
    EXPECT_EQ(std::string(req.body.begin(), req.body.end()), expected_body);
}

TEST_F(RequestParserTest, HandlesFragmentedRequest)
{
    const std::string expected_body = "data=chunked&test";
    const std::string header_line = "Content-Length: " + std::to_string(expected_body.length()) + "\r\n";

    std::vector<std::string> chunks = {
        "POST /submit HTTP/1.0\r\n",
        header_line,
        "Host: localhost\r\n",
        "\r\n",
        "data=",
        "chunked",
        "&test"
    };

    // 前几段应该都是 Parsing
    for (size_t i = 0; i + 1 < chunks.size(); ++i) {
        auto res = parser->parse(chunks[i]);
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(res.value(), State::Parsing) << "chunk idx " << i;
    }
    // 最后一段完成
    auto res = parser->parse(chunks.back());
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), State::Completed);

    const auto& req = parser->getRequest();
    EXPECT_EQ(req.method, http::Method::POST);
    EXPECT_EQ(req.uri, "/submit");
    EXPECT_EQ(req.headers.at("Host"), "localhost");
    EXPECT_EQ(req.headers.at("Content-Length"), std::to_string(expected_body.length()));
    EXPECT_EQ(std::string(req.body.begin(), req.body.end()), expected_body);
}

TEST_F(RequestParserTest, HandlesIncompleteBody)
{
    const std::string part1 = "only 10 bytes";
    const std::string part2 = "..and more";
    const std::string full_body = part1 + part2;
    const size_t content_length = full_body.length();

    const std::string header_str = "POST /incomplete HTTP/1.0\r\n"
                                   "Content-Length: "
        + std::to_string(content_length) + "\r\n\r\n";

    // 第一部分只够 Parsing
    {
        auto res = parser->parse(header_str + part1);
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(res.value(), State::Parsing);
    }
    // 第二部分触发 Completed
    {
        auto res = parser->parse(part2);
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(res.value(), State::Completed);
    }

    const auto& req = parser->getRequest();
    EXPECT_EQ(std::string(req.body.begin(), req.body.end()), full_body);
}

TEST_F(RequestParserTest, ResetFunctionality)
{
    // 第一个 GET 完成
    {
        auto res = parser->parse("GET /first HTTP/1.0\r\n\r\n");
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(res.value(), State::Completed);
        EXPECT_EQ(parser->getRequest().uri, "/first");
    }
    // reset 后应重新开始
    parser->reset();
    {
        auto res = parser->parse("POST /second HTTP/1.0\r\nContent-Length: 3\r\n\r\n123");
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(res.value(), State::Completed);

        const auto& req = parser->getRequest();
        EXPECT_EQ(req.method, http::Method::POST);
        EXPECT_EQ(req.uri, "/second");
        EXPECT_EQ(std::string(req.body.begin(), req.body.end()), "123");
    }
}

// ——— 错误处理场景 ———

TEST_F(RequestParserTest, HandlesInvalidRequestLine)
{
    auto res = parser->parse("GET /missing_version\r\n\r\n");
    // 这种情况下 parse 会返回 unexpected，has_value()==false
    ASSERT_FALSE(res.has_value());
    // （可选）检查具体错误码
    EXPECT_EQ(res.error(), http::StatusCode::BadRequest);
}

TEST_F(RequestParserTest, HandlesInvalidHeader)
{
    auto res = parser->parse("GET / HTTP/1.0\r\nHost without colon\r\n\r\n");
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), http::StatusCode::BadRequest);
}

TEST_F(RequestParserTest, HandlesInvalidContentLength)
{
    auto res = parser->parse("POST /submit HTTP/1.0\r\nContent-Length: abc\r\n\r\nabc");
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), http::StatusCode::BadRequest);
}

TEST_F(RequestParserTest, HandlesRequestWithNoDoubleCRLF)
{
    auto res = parser->parse("GET / HTTP/1.0\r\nHost: example.com");
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), State::Parsing);
}

// ===============================
// --- Transfer-Encoding: chunked 测试 ---
// ===============================

TEST_F(RequestParserTest, HandlesSimpleChunkedRequest)
{
    const std::string request_str = "POST /chunked-data HTTP/1.1\r\n"
                                    "Host: example.com\r\n"
                                    "Transfer-Encoding: chunked\r\n\r\n"
                                    "7\r\n" // 块 1: 7 字节
                                    "Mozilla\r\n"
                                    "9\r\n" // 块 2: 9 字节
                                    "Developer\r\n"
                                    "0\r\n" // 结束块
                                    "\r\n";

    auto res = parser->parse(request_str);
    ASSERT_TRUE(res.has_value()) << "Parser failed with: " << magic_enum::enum_name(res.error());
    EXPECT_EQ(res.value(), State::Completed);

    const auto& req = parser->getRequest();
    EXPECT_EQ(req.method, http::Method::POST);

    const std::string expected_body = "MozillaDeveloper";
    EXPECT_EQ(std::string(req.body.begin(), req.body.end()), expected_body);
    // 对于 chunked 请求，不应该有 Content-Length 头
    EXPECT_FALSE(req.headers.contains("Content-Length"));
}

TEST_F(RequestParserTest, HandlesFragmentedChunkedRequest)
{
    // 模拟数据被分成多个 TCP 包到达
    std::vector<std::string> chunks = {
        "POST /chunked-data HTTP/1.1\r\n",
        "Host: example.com\r\n",
        "Transfer-Encoding: chunked\r\n\r\n",
        "4\r\n", // 第一个块的大小
        "Wiki\r\n", // 第一个块的数据
        "5\r\n", // 第二个块的大小
        "pedia\r", // 第二个块的数据 (不完整)
        "\n", // 第二个块的 \n
        "E\r\n", // 第三个块的大小 (E = 14)
        " in\r\n\r\nchunks.\r", // 第三个块的数据 (跨越多行)
        "\n",
        "0\r\n\r\n" // 结束块
    };

    // 前几段应该都是 Parsing
    for (size_t i = 0; i < chunks.size() - 1; ++i) {
        auto res = parser->parse(chunks[i]);
        ASSERT_TRUE(res.has_value()) << "Parser failed at chunk " << i;
        EXPECT_EQ(res.value(), State::Parsing);
    }

    // 最后一段数据应该完成解析
    auto res = parser->parse(chunks.back());
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), State::Completed);

    const auto& req = parser->getRequest();
    const std::string expected_body = "Wikipedia in\r\n\r\nchunks.";
    EXPECT_EQ(std::string(req.body.begin(), req.body.end()), expected_body);
}

TEST_F(RequestParserTest, RejectsInvalidChunkedFormat)
{
    // 无效的十六进制大小
    const std::string bad_size_req = "POST /bad HTTP/1.1\r\n"
                                     "Transfer-Encoding: chunked\r\n\r\n"
                                     "G\r\nInvalid\r\n";

    // 块大小与实际数据长度不匹配
    const std::string mismatch_size_req = "POST /bad HTTP/1.1\r\n"
                                          "Transfer-Encoding: chunked\r\n\r\n"
                                          "10\r\n" // 声明 16 字节
                                          "only 12 bytes\r\n" // 实际只有 12 字节
                                          "0\r\n\r\n";

    parser->reset();
    auto res1 = parser->parse(bad_size_req);
    ASSERT_FALSE(res1.has_value());
    EXPECT_EQ(res1.error(), http::StatusCode::BadRequest);

    parser->reset();
    // 对于大小不匹配，我们的实现会在等待更多数据时超时，
    // 但在单元测试中，它会因为在数据结束后找不到 CRLF 而报错
    auto res2 = parser->parse(mismatch_size_req);
    ASSERT_FALSE(res2.has_value());
    EXPECT_EQ(res2.error(), http::StatusCode::BadRequest);
}

TEST_F(RequestParserTest, EnforcesBodySizeLimitOnChunkedRequest)
{
    // 创建一个解析器，限制 body 为 10 字节
    parser = std::make_unique<http::RequestParser>(10);

    const std::string request_str = "POST /too-large HTTP/1.1\r\n"
                                    "Transfer-Encoding: chunked\r\n\r\n"
                                    "5\r\n" // 块 1: 5 字节
                                    "12345\r\n"
                                    "5\r\n" // 块 2: 5 字节 (总共 10 字节，刚好)
                                    "67890\r\n"
                                    "1\r\n" // 块 3: 1 字节 (总共 11 字节，超限！)
                                    "a\r\n"
                                    "0\r\n\r\n";

    auto res = parser->parse(request_str);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), http::StatusCode::PayloadTooLarge);
}