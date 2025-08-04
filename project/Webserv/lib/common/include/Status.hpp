#pragma once
#include <cstdint>
#include <fmt/core.h>
#include <magic_enum/magic_enum.hpp>
#include <type_traits>

/**
 * @file Status.hpp
 * @brief 项目中状态枚举的统一定义（Single Source of Truth）。
 *
 * 本文件包含连接状态、IO 操作结果、协议状态等枚举，
 * 以降低认知负担并统一日志输出格式。
 */

namespace core {

/// @brief Connection 的状态机
enum class ConnectionState : uint8_t {
    READING, ///< 等待接收数据
    WRITING, ///< 等待发送数据
    CLOSING, ///< 正在关闭（写端关闭，等待对方 FIN）
    CLOSED ///< 连接已完全关闭
};

/// @brief Connection 感兴趣的事件类型（位掩码）
enum class EventType : uint8_t {
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1
};

/// @brief 支持 EventType 的按位或运算
constexpr EventType operator|(EventType lhs, EventType rhs)
{
    return static_cast<EventType>(
        static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

/// @brief 读取操作的返回结果
struct ReadResult {
    enum class Status : uint8_t {
        GotData, ///< 成功读取数据
        WouldBlock, ///< 当前无数据可读（EAGAIN / EWOULDBLOCK）
        Eof ///< 对端已关闭连接
    };
    Status status;
    std::size_t bytes_read;
};

/// @brief 写入操作的返回结果
struct WriteResult {
    enum class Status : uint8_t {
        Finished, ///< 数据已全部发送
        Continue, ///< 数据未写完，需要继续写
    };
    Status status;
    std::size_t bytes_sent;
};

namespace protocol {

    /// @brief 协议处理器的运行状态
    enum class Status : uint8_t {
        WantRead, ///< 协议需要更多输入
        WantWrite, ///< 协议已生成响应
        Finished, ///< 协议交互已完成
        Error ///< 协议处理出错
    };

} // namespace protocol
} // namespace core

namespace http {
enum class Method : uint8_t {
    GET,
    POST,
    DELETE
};
}

// ------------------------- 格式化支持区域 -------------------------------
namespace fmt {

/// @brief 通用的 magic_enum 枚举格式化器（排除位掩码类型）
template <typename T>
    requires std::is_enum_v<T> && (!std::is_same_v<T, core::EventType>)
struct formatter<T> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(T value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(value));
    }
};

/// @brief 为位掩码类型 core::EventType 提供自定义格式化器
template <>
struct formatter<core::EventType> {
    static constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(core::EventType event, FormatContext& ctx) const
    {
        if (event == core::EventType::None) {
            return fmt::format_to(ctx.out(), "None");
        }

        std::string s;
        if ((event | core::EventType::Read) == event)
            s += "Read";
        if ((event | core::EventType::Write) == event) {
            if (!s.empty())
                s += "|";
            s += "Write";
        }

        return fmt::format_to(ctx.out(), "{}", s);
    }
};

} // namespace fmt
