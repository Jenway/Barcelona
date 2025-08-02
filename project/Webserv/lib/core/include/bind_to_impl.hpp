#pragma once
#include "Channel.hpp"
#include "Poller.hpp"
#include "bind_to.hpp"
#include <expected>
#include <sys/epoll.h>

inline auto tag_invoke(bind_to_t /*unused*/, Channel& ch, Poller& poller)
    -> std::expected<void, std::system_error>
{
    if (auto res = poller.addFd(ch.fd()); !res) {
        return res;
    }

    if (ch.readableHandler()) {
        if (auto res = poller.registerCallback(ch.fd(), EPOLLIN, ch.readableHandler()); !res) {
            return res;
        }
    }

    if (ch.writableHandler()) {
        if (auto res = poller.registerCallback(ch.fd(), EPOLLOUT, ch.writableHandler()); !res) {
            return res;
        }
    }

    return {};
}
