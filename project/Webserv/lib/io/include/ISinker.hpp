#pragma once

#include "Status.hpp"
#include <expected>
#include <system_error>

/**
 * @class ISinker
 * @brief 一个纯虚基类，定义了“发送器”的接口，负责执行底层的写 I/O 操作。
 *
 * Sinker 的具体实现（如 TcpSinker, SslSinker）知道如何将一个抽象的
 * IResponse 对象转换为字节流并写入套接字。
 */
class ISinker {
public:
    virtual ~ISinker() = default;

    /**
     * @brief 写入一段在内存中的数据。
     */
    virtual auto write(const char* data, size_t len)
        -> std::expected<core::WriteStatus, std::error_code>
        = 0;

    /**
     * @brief 使用零拷贝技术发送一个文件。
     */
    virtual auto sendfile(int in_fd, size_t count)
        -> std::expected<core::WriteStatus, std::error_code>
        = 0;

    // 可以在这里添加其他 I/O 原语，比如 writev (for scatter-gather I/O)

protected:
    ISinker() = default;
    ISinker(const ISinker&) = delete;
    auto operator=(const ISinker&) -> ISinker& = delete;
    ISinker(ISinker&&) = delete;
    auto operator=(ISinker&&) -> ISinker& = delete;
};