#pragma once
#include "Channel.hpp"
#include "Poller.hpp"
#include "Status.hpp"
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
    core::EventType events = core::EventType::None;
    if (ch.readableHandler()) {
        if (auto res = poller.registerCallback(ch.fd(), core::EventType::Read, ch.readableHandler()); !res) {
            return std::unexpected(res.error());
        }
        events = events | core::EventType::Read;
    }
    if (ch.writableHandler()) {
        if (auto res = poller.registerCallback(ch.fd(), core::EventType::Write, ch.writableHandler()); !res) {
            return std::unexpected(res.error());
        }
        // 通常我们不在一开始就监听写事件，除非 Channel 特别指示
        // events = events | core::EventType::Write;
    }

    // 步骤 3: 如果有任何事件需要监听，就更新 Poller
    if (events != core::EventType::None) {
        return poller.updateEvents(ch.fd(), events);
    }

    return {};
}
