// lib/core/src/Poller.cc
#include "Poller.hpp"
#include "Error.hpp"

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

std::expected<void, std::system_error> Poller::registerCallback(int fd, uint32_t eventMask, std::function<void()> cb)
{
    callbacks_[fd][eventMask] = std::move(cb);

    epoll_event ev {};
    ev.events = 0;
    for (const auto& [mask, _] : callbacks_[fd]) {
        ev.events |= mask;
    }
    ev.data.fd = fd;

    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        return error::to_unexpected("epoll_ctl(MOD)");
    }

    return {};
}

std::expected<void, std::system_error> Poller::pollOnce(int timeoutMs)
{
    constexpr size_t MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];

    int n = epoll_wait(epollFd_, events, MAX_EVENTS, timeoutMs);
    if (n < 0) {
        return error::to_unexpected("epoll_wait");
    }

    for (int i = 0; i < n; ++i) {
        int fd = events[i].data.fd;
        uint32_t ev = events[i].events;
        if (errno == EINTR) {
            return {};
        }
        if (callbacks_.count(fd) == 0) {
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