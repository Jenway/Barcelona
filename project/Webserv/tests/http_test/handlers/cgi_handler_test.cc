#include "http/common/HttpStatus.hpp"
#include "http/common/Message.hpp"
#include "http/handlers/CgiHandler.hpp"
#include "http/interfaces/ICgiRunner.hpp"

#include <expected>
#include <functional>
#include <gtest/gtest.h>
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <system_error>

using http::CgiHandler;
using http::ICgiRunner;
using http::Request;
using http::Response;
using http::StatusCode;

// ===============================
// Mock Runner
// ===============================
class MockCgiRunner : public ICgiRunner {
public:
    std::function<std::expected<std::string, std::error_code>(
        const Request&, const std::filesystem::path&, const std::string&)>
        behavior;

    auto run(const Request& request,
        const std::filesystem::path& path,
        const std::string& interp) -> std::expected<std::string, std::error_code> override
    {
        return behavior(request, path, interp);
    }
};

// ===============================
// Test Fixture: CgiHandlerTest
// ===============================
class CgiHandlerTest : public ::testing::Test {
protected:
    std::shared_ptr<MockCgiRunner> runner;
    std::unique_ptr<CgiHandler> handler;

    void SetUp() override
    {
        runner = std::make_shared<MockCgiRunner>();
        handler = std::make_unique<CgiHandler>("/mock/root", "/bin/fake", runner);
    }

    Response callHandler(const std::string& uri)
    {
        Request req;
        req.uri = uri;
        auto result = handler->handleRequest(req);
        EXPECT_TRUE(result.has_value());
        return *result;
    }
};

static std::string getBodyString(const http::Response& resp)
{
    if (const auto* vec = std::get_if<std::vector<char>>(&resp.body)) {
        return std::string(vec->begin(), vec->end());
    }
    return "<FileBody>";
}

// ===============================
// Actual Test Cases
// ===============================

TEST_F(CgiHandlerTest, Returns502OnScriptError)
{
    runner->behavior = [](const Request&, const std::filesystem::path&, const std::string&) {
        return std::unexpected(std::make_error_code(std::errc::executable_format_error));
    };

    Request req;
    req.uri = "/fail.cgi";
    auto result = handler->handleRequest(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status_code, static_cast<int>(StatusCode::BadGateway));
}

TEST_F(CgiHandlerTest, ReturnsErrorIfCgiOutputMissingDelimiter)
{
    runner->behavior = [](const Request&, const std::filesystem::path&, const std::string&) {
        return std::string("Content-Type: text/plain\nOops no CRLF CRLF");
    };

    Request req;
    req.uri = "/badformat.cgi";
    auto result = handler->handleRequest(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status_code, magic_enum::enum_integer(http::StatusCode::BadGateway));
}

TEST_F(CgiHandlerTest, HandlesMinimalValidCgiOutput)
{
    runner->behavior = [](const Request&, const std::filesystem::path&, const std::string&) {
        return std::string("Content-Type: text/plain\r\n\r\nHello");
    };

    auto resp = callHandler("/ok.cgi");
    EXPECT_EQ(resp.status_code, 200);
    EXPECT_EQ(resp.headers["Content-Type"], "text/plain");
    EXPECT_EQ(getBodyString(resp), "Hello");
}

TEST_F(CgiHandlerTest, HandlesCgiOutputWithExplicitStatus)
{
    runner->behavior = [](const Request&, const std::filesystem::path&, const std::string&) {
        return std::string("Status: 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found");
    };

    auto resp = callHandler("/notfound.cgi");
    EXPECT_EQ(resp.status_code, 404);
    EXPECT_EQ(resp.headers["Content-Type"], "text/plain");
    EXPECT_EQ(getBodyString(resp), "Not Found");
}

TEST_F(CgiHandlerTest, HandlesMultipleHeaders)
{
    runner->behavior = [](const Request&, const std::filesystem::path&, const std::string&) {
        return std::string(
            "Content-Type: text/html\r\n"
            "Set-Cookie: a=1\r\n"
            "Set-Cookie: b=2\r\n"
            "\r\n"
            "<html></html>");
    };

    auto resp = callHandler("/cookies.cgi");
    EXPECT_EQ(resp.status_code, 200);
    EXPECT_EQ(resp.headers["Content-Type"], "text/html");
    EXPECT_EQ(getBodyString(resp), "<html></html>");
}
