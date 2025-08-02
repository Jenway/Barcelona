// in lib/logger/include/logger.hpp

#pragma once

#include <chrono>
#include <source_location>
#include <string>
#include <system_error>
#include <thread>
#include <utility>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

// Compile-time log level threshold
// 0: TRACE, 1: DEBUG, 2: INFO, 3: WARN, 4: ERROR
#ifndef LOG_LEVEL_THRESHOLD
#define LOG_LEVEL_THRESHOLD 0
#endif

// Define PROJECT_ROOT_DIR at compile time
#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR ""
#endif

namespace Logger {

enum class LogLevel : int8_t {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
};

namespace detail {
    // --- 全局设置 ---
    inline auto s_runtimeLogLevel = static_cast<LogLevel>(LOG_LEVEL_THRESHOLD);
    inline constexpr auto s_compilationLogLevel = static_cast<LogLevel>(LOG_LEVEL_THRESHOLD);
    inline bool s_enableColorOutput = true;

    // --- 辅助函数 ---
    constexpr auto levelToStyle(LogLevel level) -> fmt::text_style
    {
        switch (level) {
        case LogLevel::TRACE:
            return fg(fmt::color::gray);
        case LogLevel::DEBUG:
            return fg(fmt::color::cyan);
        case LogLevel::INFO:
            return fg(fmt::color::green);
        case LogLevel::WARN:
            return fg(fmt::color::yellow);
        case LogLevel::ERROR:
            return fg(fmt::color::red) | fmt::emphasis::bold;
        default:
            return {};
        }
    }

    constexpr auto levelToString(LogLevel level) -> std::string_view
    {
        switch (level) {
        case LogLevel::TRACE:
            return "TRACE";
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO ";
        case LogLevel::WARN:
            return "WARN ";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNDEF";
        }
    }

    inline auto getRelativePath(std::string_view absolutePath) -> std::string_view
    {
        const std::string_view projectRoot(PROJECT_ROOT_DIR);
        if (!projectRoot.empty() && absolutePath.starts_with(projectRoot)) {
            return absolutePath.substr(projectRoot.length());
        }
        return absolutePath;
    }

    inline auto shortFunctionName(std::string_view full_name) -> std::string_view
    {
        size_t paren_pos = full_name.rfind('(');
        if (paren_pos == std::string_view::npos) {
            return full_name;
        }

        size_t space_pos = full_name.rfind(' ', paren_pos);
        if (space_pos != std::string_view::npos) {
            return full_name.substr(space_pos + 1, paren_pos - (space_pos + 1));
        }

        return full_name.substr(0, paren_pos);
    }

    inline auto format_timestamp() -> std::string
    {
        auto const now = std::chrono::system_clock::now();
        auto const now_t = std::chrono::system_clock::to_time_t(now);

        std::tm tm_buf {};
        localtime_r(&now_t, &tm_buf);

        auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        return fmt::format("{:%H:%M:%S}.{:03d}", tm_buf, ms.count());
    }

    template <LogLevel Level, typename... Args>
    void log_core(std::source_location location, fmt::format_string<Args...> fmt_str, Args&&... args)
    {
        if constexpr (Level < s_compilationLogLevel) {
            return;
        }
        if (Level < s_runtimeLogLevel) {
            return;
        }

        auto& out = (Level >= LogLevel::ERROR) ? stderr : stdout;

        if (s_enableColorOutput) {
            fmt::print(out,
                "{} [{}] [{}:{}] ({})\n    {}\n",
                fmt::styled(format_timestamp(), fg(fmt::color::gray)),
                fmt::styled(levelToString(Level), levelToStyle(Level)),
                fmt::styled(getRelativePath(location.file_name()), fg(fmt::color::light_sky_blue)),
                location.line(),
                fmt::styled(shortFunctionName(location.function_name()), fg(fmt::color::light_yellow)),
                fmt::format(fmt_str, std::forward<Args>(args)...));
        } else {
            fmt::print(out,
                "{} [{}] [{}:{}] ({})\n    {}\n",
                format_timestamp(),
                levelToString(Level),
                getRelativePath(location.file_name()),
                location.line(),
                shortFunctionName(location.function_name()),
                fmt::format(fmt_str, std::forward<Args>(args)...));
        }
    }
} // namespace detail

// --- 公共 API ---
inline void setRuntimeLogLevel(LogLevel level) { Logger::detail::s_runtimeLogLevel = level; }
inline void enableColor(bool enable) { Logger::detail::s_enableColorOutput = enable; }

} // namespace Logger

// --- 日志宏定义 ---
// 使用 FMT_STRING() 进行编译期格式化字符串检查
// 使用 ##__VA_ARGS__ 来处理没有可变参数的情况（例如 LOG_INFO("Server started")）
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define LOG_TRACE(fmt_str, ...) \
    Logger::detail::log_core<Logger::LogLevel::TRACE>(std::source_location::current(), FMT_STRING(fmt_str), ##__VA_ARGS__)
#define LOG_DEBUG(fmt_str, ...) \
    Logger::detail::log_core<Logger::LogLevel::DEBUG>(std::source_location::current(), FMT_STRING(fmt_str), ##__VA_ARGS__)
#define LOG_INFO(fmt_str, ...) \
    Logger::detail::log_core<Logger::LogLevel::INFO>(std::source_location::current(), FMT_STRING(fmt_str), ##__VA_ARGS__)
#define LOG_WARN(fmt_str, ...) \
    Logger::detail::log_core<Logger::LogLevel::WARN>(std::source_location::current(), FMT_STRING(fmt_str), ##__VA_ARGS__)
#define LOG_ERROR(fmt_str, ...) \
    Logger::detail::log_core<Logger::LogLevel::ERROR>(std::source_location::current(), FMT_STRING(fmt_str), ##__VA_ARGS__)
// NOLINTEND(cppcoreguidelines-macro-usage)

template <>
struct fmt::formatter<std::error_code> {
    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin(); // no format specifiers supported for now
    }

    template <typename FormatContext>
    auto format(const std::error_code& ec, FormatContext& ctx) const -> FormatContext::iterator
    {
        return fmt::format_to(ctx.out(), "[{}: {}]", ec.category().name(), ec.message());
    }
};

template <>
struct fmt::formatter<std::thread::id> {
    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin(); // no format specifiers supported for now
    }

    template <typename FormatContext>
    auto format(const std::thread::id& id, FormatContext& ctx) const -> FormatContext::iterator
    {
        return fmt::format_to(ctx.out(), "{}", id);
    }
};
