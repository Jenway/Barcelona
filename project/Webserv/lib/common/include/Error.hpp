#pragma once

#include <expected>
#include <fmt/core.h>
#include <source_location>
#include <string>
#include <string_view>
#include <system_error>

namespace error {

struct diagnostic_context {
    std::error_code ec;
    std::string msg;
    std::source_location loc = std::source_location::current();
    // future extension: module, hint, trace_id, etc.
};

inline constexpr struct to_unexpected_t {
    template <typename... Args>
    auto operator()(Args&&... args) const
        noexcept(noexcept(tag_invoke(*this, std::forward<Args>(args)...)))
            -> decltype(tag_invoke(*this, std::forward<Args>(args)...))
    {
        return tag_invoke(*this, std::forward<Args>(args)...);
    }
} to_unexpected;

inline constexpr struct to_unexpected_code_t {
    template <typename... Args>
    auto operator()(Args&&... args) const
        noexcept(noexcept(tag_invoke(*this, std::forward<Args>(args)...)))
            -> decltype(tag_invoke(*this, std::forward<Args>(args)...))
    {
        return tag_invoke(*this, std::forward<Args>(args)...);
    }
} to_unexpected_code;

inline auto tag_invoke(to_unexpected_t /*unused*/, diagnostic_context const& ctx)
    -> std::unexpected<std::system_error>
{
    return std::unexpected(std::system_error(ctx.ec,
        fmt::format(
            "[{}:{}] {} ({}: {})",

            std::string_view(ctx.loc.file_name()).substr(std::string_view(ctx.loc.file_name()).find_last_of('/') + 1),
            ctx.loc.line(),
            ctx.msg,
            ctx.ec.category().name(),
            ctx.ec.value())));
}

// 重载 1: 接收 const char*, std::string, std::string_view
template <typename T>
auto tag_invoke(to_unexpected_t /*unused*/, std::error_code ec, T&& msg)
    -> std::unexpected<std::system_error>
    requires(std::is_convertible_v<T, std::string>)
{
    return to_unexpected(diagnostic_context {
        .ec = ec,
        .msg = std::string(std::forward<T>(msg)) });
}

// 重载 2: 处理系统 errno
inline auto tag_invoke(to_unexpected_t /*unused*/, char const* msg)
    -> std::unexpected<std::system_error>
{
    return to_unexpected(diagnostic_context {
        .ec = { errno, std::generic_category() },
        .msg = msg,
    });
}

// 重载 3: 只接收 error_code
inline auto tag_invoke(to_unexpected_t /*unused*/, std::error_code ec)
    -> std::unexpected<std::system_error>
{
    return to_unexpected(ec, ec.message());
}

// 重载 4: 无参数版本
inline auto tag_invoke(to_unexpected_t /*unused*/)
    -> std::unexpected<std::system_error>
{
    auto error_code = std::error_code { errno, std::generic_category() };
    return to_unexpected(error_code, error_code.message());
}

inline auto tag_invoke(to_unexpected_code_t /*unused*/)
    -> std::unexpected<std::error_code>
{
    return std::unexpected(std::error_code { errno, std::generic_category() });
}
} // namespace error
