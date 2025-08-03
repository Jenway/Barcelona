#pragma once

#include <expected>
#include <fmt/core.h>
#include <source_location>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace error {
namespace detail {
    template <typename T>
    auto tag_invoke_impl(std::error_code ec, T&& msg, std::source_location loc)
        -> std::unexpected<std::system_error>
    {
        std::string context_info = fmt::format("[{}:{}] {}",
            std::string_view(loc.file_name()).substr(std::string_view(loc.file_name()).find_last_of('/') + 1),
            loc.line(),
            std::forward<T>(msg));
        return std::unexpected(std::system_error(ec, context_info));
    }
} // namespace detail

inline constexpr struct to_unexpected_t {
    // 重载 1: (msg, [loc])
    auto operator()(const char* msg, std::source_location loc = std::source_location::current()) const
    {
        return detail::tag_invoke_impl({ errno, std::generic_category() }, msg, loc);
    }

    // 重载 2: (ec, msg, [loc])
    template <typename T>
    auto operator()(std::error_code ec, T&& msg, std::source_location loc = std::source_location::current()) const
        requires(std::is_convertible_v<T, std::string_view>)
    {
        return detail::tag_invoke_impl(ec, std::forward<T>(msg), loc);
    }
    template <typename T>
    auto operator()(std::errc ec, T&& msg, std::source_location loc = std::source_location::current()) const
        requires(std::is_convertible_v<T, std::string_view>)
    {
        auto code = std::make_error_code(ec);
        return detail::tag_invoke_impl(code, std::forward<T>(msg), loc);
    }

    // 重载 3: (ec, [loc])
    auto operator()(std::error_code ec, std::source_location loc = std::source_location::current()) const
    {
        return detail::tag_invoke_impl(ec, ec.message(), loc);
    }

    // 重载 4: (), [loc]
    auto operator()(std::source_location loc = std::source_location::current()) const
    {
        auto ec = std::error_code { errno, std::generic_category() };
        return detail::tag_invoke_impl(ec, ec.message(), loc);
    }

    // 重载 4: (errc), [loc]
    auto operator()(std::errc ec, std::source_location loc = std::source_location::current()) const
    {
        auto code = std::make_error_code(ec);
        return detail::tag_invoke_impl(code, code.message(), loc);
    }

} to_unexpected;

inline constexpr struct to_unexpected_code_t {
    auto operator()() const
    {
        return std::unexpected(std::error_code { errno, std::generic_category() });
    }
} to_unexpected_code;

} // namespace error