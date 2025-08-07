// tests/logger_test/logger_output_test.cc

#include "logger.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::HasSubstr;
using ::testing::MatchesRegex;
using ::testing::Not;

// ======================= 测试夹具 =======================
class LoggerOutputTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        Logger::enableColor(false);
    }

    void TearDown() override
    {
        Logger::enableColor(true);
    }
};
// ===================================================================

TEST_F(LoggerOutputTest, BasicFormatAndContent)
{
    testing::internal::CaptureStdout();
    LOG_INFO("User {} logged in successfully.", 42);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, HasSubstr("[INFO]"));
    EXPECT_THAT(output, HasSubstr("TestBody"));
    EXPECT_THAT(output, HasSubstr("User 42 logged in successfully."));
    EXPECT_THAT(output, MatchesRegex("^[0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{2} .*"));
}

TEST_F(LoggerOutputTest, ErrorGoesToStderr)
{
    testing::internal::CaptureStderr();
    LOG_ERROR("Something went terribly wrong.");
    std::string output = testing::internal::GetCapturedStderr();

    EXPECT_THAT(output, HasSubstr("[ERROR]"));
    EXPECT_THAT(output, HasSubstr("Something went terribly wrong."));
}

TEST_F(LoggerOutputTest, LogLevelFilteringIsVerified)
{
    Logger::setRuntimeLogLevel(Logger::LogLevel::WARN);

    testing::internal::CaptureStdout();
    LOG_INFO("This should be invisible.");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "");

    Logger::setRuntimeLogLevel(static_cast<Logger::LogLevel>(LOG_LEVEL_THRESHOLD));
}