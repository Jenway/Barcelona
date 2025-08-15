// lib/core/src/Poller.cc
#include "Poller.hpp"
#include "Error.hpp"
#include "Status.hpp"

#include <array>
#include <expected>
#include <sys/epoll.h>
#include <unistd.h>

Poller::Poller()
    : epollFd_(::epoll_create1(EPOLL_CLOEXEC))
{
    if (epollFd_ < 0) {
        std::abort();
    }
}
Poller::~Poller() { ::close(epollFd_); }

std::expected<void, std::system_error> Poller::addFd(int fd) const
{
    epoll_event ev {};
    ev.events = 0;
    ev.data.fd = fd;

    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        return error::to_unexpected("epoll_ctl(ADD)");
    }

    return {};
}

std::expected<void, std::system_error> Poller::removeFd(int fd)
{
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        return error::to_unexpected("epoll_ctl(DEL)");
    }
    callbacks_.erase(fd);
    return {};
}

namespace {
uint32_t to_epoll_events(core::EventType abstract_events)
{
    uint32_t concrete_events = 0;
    if ((static_cast<uint8_t>(abstract_events) & static_cast<uint8_t>(core::EventType::Read)) != 0) {
        concrete_events |= EPOLLIN;
    }
    if ((static_cast<uint8_t>(abstract_events) & static_cast<uint8_t>(core::EventType::Write)) != 0) {
        concrete_events |= EPOLLOUT;
    }
    // 未来可以扩展，例如：
    // concrete_events |= EPOLLET; // Edge-Triggered
    return concrete_events;
}
} // namespace

std::expected<void, std::system_error> Poller::updateEvents(int fd, core::EventType abstract_events) const
{
    uint32_t concrete_events = to_epoll_events(abstract_events);

    epoll_event ev {};
    ev.events = concrete_events;
    ev.data.fd = fd;

    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        return error::to_unexpected("epoll_ctl(MOD) on updateEvents");
    }
    return {};
}

std::expected<void, std::system_error> Poller::registerCallback(int fd, core::EventType abstract_events, std::function<void()> cb)
{
    uint32_t concrete_events = to_epoll_events(abstract_events);

    callbacks_[fd][concrete_events] = std::move(cb);

    return {};
}

std::expected<void, std::system_error> Poller::pollOnce(int timeoutMs)
{
    constexpr size_t MAX_EVENTS = 64;
    std::array<epoll_event, MAX_EVENTS> events {};

    int n = epoll_wait(epollFd_, events.data(), MAX_EVENTS, timeoutMs);
    if (n < 0) {
        if (errno == EINTR) {
            return {};
        }
        return error::to_unexpected("epoll_wait");
    }

    for (int i = 0; i < n; ++i) {
        auto [ev, data] = events.at(i);
        int fd = data.fd;
        if (errno == EINTR) {
            return {};
        }
        if (!callbacks_.contains(fd)) {
            continue;
        }
        auto callbacks_for_fd = callbacks_.at(fd);

        for (const auto& [mask, cb] : callbacks_for_fd) {
            if (((ev & mask) != 0U) && cb) {
                cb();
            }
        }
    }

    return {};
}