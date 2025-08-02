#pragma once
#include "Channel.hpp"
#include "Poller.hpp"
#include "bind_to.hpp"
#include <expected>
#include <sys/epoll.h>

inline auto tag_invoke(bind_to_t /*unused*/, Channel& ch, Poller& poller)
    -> std::expected<void, std::system_error>
{
    // 步骤 1: 将 fd 添加到 epoll
    if (auto res = poller.addFd(ch.fd()); !res) {
        return res;
    }

    // 步骤 2: 注册回调并计算初始事件
    uint32_t events = 0;
    if (ch.readableHandler()) {
        poller.registerCallback(ch.fd(), EPOLLIN, ch.readableHandler());
        events |= EPOLLIN;
    }
    if (ch.writableHandler()) {
        poller.registerCallback(ch.fd(), EPOLLOUT, ch.writableHandler());
        // 通常我们不在一开始就监听写事件，除非 Channel 特别指示
        // events |= EPOLLOUT;
    }

    // 步骤 3: 如果有任何事件需要监听，就更新 Poller
    if (events != 0) {
        return poller.updateEvents(ch.fd(), events);
    }

    return {};
}
