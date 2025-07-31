#pragma once
#include "Logger.hpp"
#include <cstring>
#include <expected>
#include <fcntl.h>
#include <system_error>
#include <unistd.h>
#include <utility>

class FileDescriptor {
private:
    int _fd = -1;
    bool _non_block = true;

    FileDescriptor(int fd, bool non_block)
        : _fd(fd)
        , _non_block(non_block)
    {
    }

    std::expected<void, std::system_error> configure()
    {
        if (_fd == -1 || !_non_block)
            return {};

        int flags = fcntl(_fd, F_GETFL, 0);
        if (flags == -1 || fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            int err = errno;
            ::close(_fd);
            _fd = -1;
            return std::unexpected(std::system_error(err, std::generic_category(), "Failed to set O_NONBLOCK"));
        }

        int fd_flags = fcntl(_fd, F_GETFD, 0);
        if (fd_flags == -1 || fcntl(_fd, F_SETFD, fd_flags | FD_CLOEXEC) == -1) {
            int err = errno;
            ::close(_fd);
            _fd = -1;
            return std::unexpected(std::system_error(err, std::generic_category(), "Failed to set FD_CLOEXEC"));
        }

        return {};
    }

public:
    static std::expected<FileDescriptor, std::system_error> create(int fd, bool non_block = true)
    {
        if (fd < 0) {
            return std::unexpected(std::system_error(errno, std::generic_category(), "Invalid file descriptor"));
        }

        FileDescriptor fd_obj(fd, non_block);
        if (auto result = fd_obj.configure(); !result) {
            return std::unexpected(result.error());
        }

        return fd_obj;
    }

    ~FileDescriptor()
    {
        if (_fd != -1)
            ::close(_fd);
    }

    FileDescriptor(FileDescriptor&& other) noexcept
        : _fd(std::exchange(other._fd, -1))
        , _non_block(other._non_block)
    {
    }

    FileDescriptor& operator=(FileDescriptor&& other) noexcept
    {
        if (this != &other) {
            reset();
            _fd = std::exchange(other._fd, -1);
            _non_block = other._non_block;
        }
        return *this;
    }

    void reset(int new_fd = -1)
    {
        if (_fd != -1)
            ::close(_fd);
        _fd = new_fd;
        // 你可以选择是否在 reset 中再次调用 configure
        configure();
    }

    [[nodiscard]] bool valid() const noexcept { return _fd != -1; }
    [[nodiscard]] int get() const noexcept { return _fd; }

    int release() noexcept { return std::exchange(_fd, -1); }

    explicit operator bool() const noexcept { return valid(); }
    explicit operator int() const noexcept { return _fd; }

    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    friend void swap(FileDescriptor& a, FileDescriptor& b) noexcept
    {
        std::swap(a._fd, b._fd);
        std::swap(a._non_block, b._non_block);
    }
};
