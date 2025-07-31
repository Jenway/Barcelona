#pragma once

#include "ISinker.hpp"
#include "Socket.hpp"
#include "Status.hpp"

/**
 * @class TcpSinker
 * @brief ISinker 接口的具体实现，负责向一个 TCP Socket 写入数据。
 *
 * 这个类是底层 I/O 写的执行者。它将 ISinker 接口中定义的抽象 I/O 原语
 * （如 write, sendfile）映射到具体的 POSIX 系统调用上。
 */
class TcpSinker : public ISinker {
public:
    explicit TcpSinker(Socket& socket);

    auto write(const char* data, size_t len)
        -> std::expected<core::WriteResult, std::error_code> override;

    auto sendfile(int in_fd, off_t& offset, size_t count)
        -> std::expected<core::WriteResult, std::error_code> override;

    TcpSinker(const TcpSinker&) = delete;
    auto operator=(const TcpSinker&) -> TcpSinker& = delete;
    TcpSinker(TcpSinker&&) = delete;
    auto operator=(TcpSinker&&) -> TcpSinker& = delete;

private:
    Socket& socket_;
};