#pragma once

#include "IResponse.hpp"
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
     * @brief 尝试发送一个响应对象。
     * @param response 一个指向待发送响应的 const 引用。
     * @return 一个包含 SinkResult 的 expected，或者一个 error_code。
     */
    virtual auto send(const IResponse& response) -> std::expected<core::WriteStatus, std::error_code> = 0;

protected:
    ISinker() = default;
    ISinker(const ISinker&) = delete;
    auto operator=(const ISinker&) -> ISinker& = delete;
    ISinker(ISinker&&) = delete;
    auto operator=(ISinker&&) -> ISinker& = delete;
};