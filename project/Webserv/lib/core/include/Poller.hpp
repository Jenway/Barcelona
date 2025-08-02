// lib/core/include/Poller.hpp
#pragma once
#include <expected>
#include <functional>
#include <system_error>
#include <unordered_map>

class Poller {
public:
    Poller();
    ~Poller();

    auto addFd(int fd) const -> std::expected<void, std::system_error>;
    auto removeFd(int fd) -> std::expected<void, std::system_error>;
    auto updateEvents(int fd, uint32_t events) -> std::expected<void, std::system_error>;

    auto registerCallback(int fd, uint32_t eventMask, std::function<void()> cb) -> std::expected<void, std::system_error>;

    auto pollOnce(int timeoutMs = -1) -> std::expected<void, std::system_error>;

private:
    int epollFd_ = -1;
    std::unordered_map<int, std::unordered_map<uint32_t, std::function<void()>>> callbacks_;
};
