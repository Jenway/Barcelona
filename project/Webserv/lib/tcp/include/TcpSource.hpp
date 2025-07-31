#pragma once
#include "ISource.hpp"
#include "Socket.hpp"

/**
 * @class TcpSource
 * @brief ISource 接口的具体实现，负责从一个 TCP Socket 读取数据。
 * 这个类封装了对底层 ::read() 系统调用的调用，并将其结果转换为我们定义的、更高级的 ReadStatus 枚举。
 */
class TcpSource : public ISource {
public:
    explicit TcpSource(Socket& socket);

    auto read(std::vector<char>& buffer)
        -> std::expected<std::pair<core::ReadStatus, size_t>, std::error_code> override;

    TcpSource(const TcpSource&) = delete;
    auto operator=(const TcpSource&) -> TcpSource& = delete;
    TcpSource(TcpSource&&) = delete;
    auto operator=(TcpSource&&) -> TcpSource& = delete;

private:
    Socket& socket_;
};