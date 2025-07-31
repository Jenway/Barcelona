#pragma once
#include <cstdint>

/**
 * @file Status.hpp
 * @brief 定义了服务器核心逻辑中使用的所有状态和结果枚举。
 *
 * 这个文件是项目状态管理的“单一事实来源 (Single Source of Truth)”，
 * 旨在通过统一的命名和组织，降低开发者的认知负荷。
 */

// 命名空间 core 将包含所有核心逻辑相关的定义
namespace core {

/**
 * @enum ConnectionState
 * @brief 描述了一个 Connection 对象的内部状态机。
 */
enum class ConnectionState : uint8_t {
    READING, ///< 正在等待接收数据。
    WRITING, ///< 正在等待发送数据。
    CLOSING, ///< Gracefully shutting down (our write-end is closed, waiting for client FIN).
    CLOSED, ///< 连接已完全关闭。
};

/**
 * @enum ReadStatus
 * @brief 描述了从一个 ISource 读取操作的结果。
 */
enum class ReadStatus : uint8_t {
    GotData, ///< 成功读取到数据。
    WouldBlock, ///< 当前无数据可读 (EAGAIN / EWOULDBLOCK)。
    Eof, ///< 对端已关闭连接。
};

/**
 * @enum WriteResult
 * @brief 描述了向一个 ISinker 写入操作的结果。
 */
struct WriteResult {
    enum Status : uint8_t {
        Finished, ///< 响应已全部发送完毕。
        Continue, ///< 响应只发送了一部分，需要继续写。
    };

    Status status; // 本次写入的状态
    std::size_t bytes_sent; // 本次写入成功发送的字节数
};

// -----------------------------------------------------

// 命名空间 protocol 专门用于协议处理器相关的定义
namespace protocol {
    /**
     * @enum Status
     * @brief 描述了协议处理器的期望状态，用于指导 Connection。
     */
    enum class Status : uint8_t {
        WantRead, ///< 协议需要更多数据。
        WantWrite, ///< 协议已准备好一个响应。
        Finished, ///< 协议交换完成。
        Error, ///< 协议出错。
    };
} // namespace protocol

} // namespace core
