// in lib/utils/include/FileDescriptor.hpp
#pragma once

#include <unistd.h>

namespace utils {

class FileDescriptor {
public:
    explicit FileDescriptor(int fd = -1)
        : _fd(fd)
    {
    }

    ~FileDescriptor()
    {
        if (isValid()) {
            ::close(_fd);
        }
    }

    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    FileDescriptor(FileDescriptor&& other) noexcept
        : _fd(other._fd)
    {
        other._fd = -1;
    }

    FileDescriptor& operator=(FileDescriptor&& other) noexcept
    {
        if (this != &other) {
            if (isValid()) {
                ::close(_fd);
            }
            _fd = other._fd;
            other._fd = -1;
        }
        return *this;
    }

    [[nodiscard]] bool isValid() const { return _fd != -1; }
    [[nodiscard]] int get() const { return _fd; }

    // 释放所有权（当我们将 fd 交给其他不使用 RAII 的组件时使用）
    [[nodiscard]] int release()
    {
        int temp_fd = _fd;
        _fd = -1;
        return temp_fd;
    }

private:
    int _fd = -1;
};

} // namespace utils