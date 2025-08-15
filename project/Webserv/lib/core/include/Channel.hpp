#pragma once
#include <functional>

class Channel {
public:
    explicit Channel(int fd)
        : fd_(fd)
    {
    }

    void setReadableHandler(std::function<void()> cb) { onReadable_ = std::move(cb); }
    void setWritableHandler(std::function<void()> cb) { onWritable_ = std::move(cb); }

    [[nodiscard]] int fd() const { return fd_; }

    [[nodiscard]] const std::function<void()>& readableHandler() const { return onReadable_; }
    [[nodiscard]] const std::function<void()>& writableHandler() const { return onWritable_; }

private:
    int fd_;
    std::function<void()> onReadable_;
    std::function<void()> onWritable_;
};
