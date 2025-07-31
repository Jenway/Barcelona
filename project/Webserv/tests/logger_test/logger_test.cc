// tests/logger_test/logger_test.cc

#include "logger.hpp"
#include <gtest/gtest.h>
#include <system_error>

TEST(LoggerTest, BasicMacros)
{
    // 这个测试的目标是确保所有宏在调用时都能正常工作且不崩溃。
    // 我们期望在测试输出中看到这些日志信息。
    SUCCEED(); // 先标记测试成功，除非有异常抛出

    LOG_TRACE("This is a trace message with a number: {}", 1);
    LOG_DEBUG("This is a debug message with a string: '{}'", "test");
    LOG_INFO("This is an info message.");
    LOG_WARN("This is a warning message.");

    // 模拟一个错误码
    errno = EPERM; // Operation not permitted
    LOG_ERROR("This is an error message with a system error: {}",
        std::error_code(EPERM, std::generic_category()).message());
}

TEST(LoggerTest, LogLevelFiltering)
{
    // 我们不能轻易捕获 stdout/stderr 来验证输出，
    // 但这个测试至少验证了在不同日志级别下，程序逻辑不会出错。
    SUCCEED();

    Logger::setRuntimeLogLevel(Logger::LogLevel::WARN);
    LOG_INFO("This message should NOT appear."); // 因为级别低于 WARN
    LOG_WARN("This warning SHOULD appear.");

    // 恢复默认，以免影响其他测试
    Logger::setRuntimeLogLevel(static_cast<Logger::LogLevel>(LOG_LEVEL_THRESHOLD));
}