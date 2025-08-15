#pragma once
#include "ISource.hpp"
#include "Status.hpp"

class TcpSource : public ISource {
public:
    TcpSource() = default;
    ~TcpSource() override = default;
    TcpSource(const TcpSource&) = delete;
    auto operator=(const TcpSource&) -> TcpSource& = delete;
    TcpSource(TcpSource&&) = delete;
    auto operator=(TcpSource&&) -> TcpSource& = delete;

    void setFd(int fd) { this->fd_ = fd; }

    auto read(std::vector<char>& buffer)
        -> std::expected<core::ReadResult, std::error_code> override;

private:
    int fd_ = -1;
};