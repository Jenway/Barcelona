// in lib/logger/include/logger.hpp

#pragma once

#include <chrono>
#include <expected>
#include <source_location>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <utility>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_containers.hpp>

// Compile-time log level threshold
// 0: TRACE, 1: DEBUG, 2: INFO, 3: WARN, 4: ERROR
#ifndef LOG_LEVEL_THRESHOLD
#define LOG_LEVEL_THRESHOLD 0
#endif

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "SET PROJECT_ROOT_DIR SO THAT I CAN WORK"
#endif

namespace Logger {

enum class LogLevel : int8_t {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
};

constexpr std::array styles = {
    fg(fmt::color::gray), // TRACE
    fg(fmt::color::cyan), // DEBUG
    fg(fmt::color::green), // INFO
    fg(fmt::color::yellow), // WARN
    fg(fmt::color::red) | fmt::emphasis::bold // ERROR
};

constexpr auto stringToLevel(std::string_view levelStr) -> std::expected<LogLevel, std::errc>
{
    auto opt = magic_enum::enum_cast<Logger::LogLevel>(levelStr);
    if (opt.has_value()) {
        return *opt;
    }
    return std::unexpected(std::errc::invalid_argument);
}

namespace detail {
    // --- 全局设置 ---
    inline constexpr auto s_compilationLogLevel = static_cast<LogLevel>(LOG_LEVEL_THRESHOLD);
    inline auto s_runtimeLogLevel = static_cast<LogLevel>(LOG_LEVEL_THRESHOLD);
    inline bool s_enableColorOutput = true;

    // --- 辅助函数 ---
    consteval auto levelToStyle(LogLevel level) -> fmt::text_style
    {
        if (auto idx = magic_enum::enum_index(level); idx.has_value() && *idx < styles.size())
            return styles[*idx];
        return {};
    }

    consteval auto levelToString(LogLevel level) -> std::string_view
    {
        if (auto name = magic_enum::enum_name(level); !name.empty()) {
            return name;
        }
        return "UNDEF";
    }

    constexpr auto getRelativePath(std::string_view absolutePath) -> std::string_view
    {
        constexpr std::string_view projectRoot = PROJECT_ROOT_DIR;
        constexpr std::string_view pr = (!projectRoot.empty() && projectRoot.back() != '/') ? projectRoot : projectRoot.substr(0, projectRoot.size());

        if (!pr.empty() && absolutePath.starts_with(pr)) {
            return absolutePath.substr(pr.length());
        }
        return absolutePath;
    }

    constexpr auto shortFunctionName(std::string_view full_name) -> std::string_view
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
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto local_tm = fmt::localtime(now_time_t);

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        int hundredths = static_cast<int>(ms.count() / 10);
        return fmt::format("{:%H:%M:%S}.{:02d}", local_tm, hundredths);
    }

    template <LogLevel Level, typename... Args>
    void log_core_impl(fmt::format_string<Args...> fmt_str,
        std::source_location location,
        Args&&... args)
    {
        if constexpr (Level < s_compilationLogLevel)
            return;
        if (Level < s_runtimeLogLevel)
            return;

        auto& out = (Level >= LogLevel::ERROR ? stderr : stdout);
        auto ts = format_timestamp();
        auto lvlStr = levelToString(Level);
        auto file = getRelativePath(location.file_name());
        auto line = location.line();
        auto func = shortFunctionName(location.function_name());
        auto message = fmt::format(fmt_str, std::forward<Args>(args)...);

        if (s_enableColorOutput) {
            fmt::print(out,
                "{} [{}] [{}:{}] ({})\n    {}\n",
                fmt::styled(ts, fg(fmt::color::gray)),
                fmt::styled(lvlStr, levelToStyle(Level)),
                fmt::styled(file, fg(fmt::color::light_sky_blue)),
                line,
                fmt::styled(func, fg(fmt::color::light_yellow)),
                message);
        } else {
            fmt::print(out,
                "{} [{}] [{}:{}] ({})\n    {}\n",
                ts, lvlStr, file, line, func, message);
        }
    }

} // namespace detail

// --- 公共 API ---
inline void setRuntimeLogLevel(LogLevel level) { Logger::detail::s_runtimeLogLevel = level; }
inline void enableColor(bool enable) { Logger::detail::s_enableColorOutput = enable; }

template <LogLevel Level, typename... Args>
inline void log(fmt::format_string<Args...> fmt_str,
    Args&&... args,
    std::source_location loc = std::source_location::current())
{
    detail::log_core_impl<Level>(fmt_str, loc, std::forward<Args>(args)...);
}
#define DEFINE_LOG_FUNCTION(func_name, log_level)                                    \
    template <typename... Args>                                                      \
    inline void func_name(fmt::format_string<Args...> fmt_str,                       \
        Args&&... args,                                                              \
        std::source_location loc = std::source_location::current())                  \
    {                                                                                \
        detail::log_core_impl<log_level>(fmt_str, loc, std::forward<Args>(args)...); \
    }

DEFINE_LOG_FUNCTION(trace, LogLevel::TRACE)
DEFINE_LOG_FUNCTION(debug, LogLevel::DEBUG)
DEFINE_LOG_FUNCTION(info, LogLevel::INFO)
DEFINE_LOG_FUNCTION(warn, LogLevel::WARN)
DEFINE_LOG_FUNCTION(error, LogLevel::ERROR)

#undef DEFINE_LOG_FUNCTION

} // namespace Logger

// --- 日志宏定义 ---
// 使用 FMT_STRING() 进行编译期格式化字符串检查
// 使用 ##__VA_ARGS__ 来处理没有可变参数的情况（例如 LOG_INFO("Server started")）
#define LOG_TRACE(fmt_str, ...) \
    Logger::detail::log_core_impl<Logger::LogLevel::TRACE>(FMT_STRING(fmt_str), std::source_location::current(), ##__VA_ARGS__)

#define LOG_DEBUG(fmt_str, ...) \
    Logger::detail::log_core_impl<Logger::LogLevel::DEBUG>(FMT_STRING(fmt_str), std::source_location::current(), ##__VA_ARGS__)

#define LOG_INFO(fmt_str, ...) \
    Logger::detail::log_core_impl<Logger::LogLevel::INFO>(FMT_STRING(fmt_str), std::source_location::current(), ##__VA_ARGS__)

#define LOG_WARN(fmt_str, ...) \
    Logger::detail::log_core_impl<Logger::LogLevel::WARN>(FMT_STRING(fmt_str), std::source_location::current(), ##__VA_ARGS__)

#define LOG_ERROR(fmt_str, ...) \
    Logger::detail::log_core_impl<Logger::LogLevel::ERROR>(FMT_STRING(fmt_str), std::source_location::current(), ##__VA_ARGS__)

template <>
struct fmt::formatter<std::error_code> {
    static constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::error_code& ec, FormatContext& ctx) const -> FormatContext::iterator
    {
        return fmt::format_to(ctx.out(), "[{}: {}]", ec.category().name(), ec.message());
    }
};

template <>
struct fmt::formatter<std::system_error> {
    static constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::system_error& se, FormatContext& ctx) const -> FormatContext::iterator
    {
        return fmt::format_to(ctx.out(),
            "\n" // 换行开始
            "        - What: {}\n"
            "        - Code: {} ({})",
            se.what(),
            se.code().value(),
            se.code().category().name());
    }
};

template <>
struct fmt::formatter<std::thread::id> {
    static constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin(); // no format specifiers supported for now
    }

    template <typename FormatContext>
    auto format(const std::thread::id& id, FormatContext& ctx) const -> FormatContext::iterator
    {
        return fmt::format_to(ctx.out(), "{}", id);
    }
};

template <>
struct fmt::formatter<Logger::LogLevel> {
    static constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Logger::LogLevel& level, FormatContext& ctx) const -> FormatContext::iterator
    {
        if (auto name = magic_enum::enum_name(level); !name.empty()) {
            return fmt::format_to(ctx.out(), "{}", name);
        }
        return fmt::format_to(ctx.out(), "Undefined");
    }
};