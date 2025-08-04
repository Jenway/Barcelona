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
