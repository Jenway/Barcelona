// in lib/tcp/include/Endpoint.hpp
#pragma once

#include "ErrorCode.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <expected>
#include <string>
#include <system_error>
#include <utility>

namespace net {

// 封装了一个 IPv4 地址和端口
class TcpEndpoint {
public:
    TcpEndpoint() = default;
    TcpEndpoint(std::string ip_address, uint16_t port)
        : _ip_address(std::move(ip_address))
        , _port(port)
    {
    }
    explicit TcpEndpoint(uint16_t port)
        : TcpEndpoint("127.0.0.1", port)
    {
    }

    [[nodiscard]] const std::string& getAddress() const { return _ip_address; }
    [[nodiscard]] uint16_t getPort() const { return _port; }

    [[nodiscard]] auto toSockAddr() const -> std::expected<sockaddr_in, std::error_code>
    {
        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_port);

        if (inet_pton(AF_INET, _ip_address.c_str(), &addr.sin_addr) <= 0) {
            return std::unexpected(ErrorCode::Net_InvalidAddress);
        }

        return addr;
    }

private:
    std::string _ip_address;
    uint16_t _port = 0;
};

} // namespace net