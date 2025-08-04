#pragma once

#include <expected>
#include <magic_enum/magic_enum.hpp>
#include <string>
#include <system_error>

// 1. 定义我们自己的 ErrorCode 枚举
enum class ErrorCode : uint8_t {
    Ok = 0,
    Unknown,
    // NET
    Net_InvalidAddress,
    // FdNotSet
    Fd_NotSet,
    // CONFIG
    Config_FileNotFound,
    Config_ParseError,
    Config_InvalidBodySize,
};

class WebServerCategory : public std::error_category {
public:
    [[nodiscard]] const char* name() const noexcept override { return "WebServer"; }

    [[nodiscard]] std::string message(int condition) const override
    {
        auto enum_val = static_cast<ErrorCode>(condition);
        auto name = magic_enum::enum_name(enum_val);
        return std::string(name);
    }
};

inline const WebServerCategory& get_web_server_category() noexcept
{
    static WebServerCategory instance;
    return instance;
}

inline std::error_code make_error_code(ErrorCode e) noexcept
{
    return { static_cast<int>(e), get_web_server_category() };
}

// 告诉 C++ 标准库，我们的 ErrorCode 可以被当作 error_code 使用
namespace std {
template <>
struct is_error_code_enum<ErrorCode> : true_type { };
} // namespace std

inline std::error_code make_system_error_code()
{
    return { errno, std::generic_category() };
}
