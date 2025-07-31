// in lib/tcp/include/FileDescriptor.hpp

#pragma once
#include <unistd.h>

class FileDescriptor {
public:
    explicit FileDescriptor(int fd = -1)
        : fd_(fd)
    {
    }

    ~FileDescriptor()
    {
        if (isValid()) {
            ::close(fd_);
        }
    }

    FileDescriptor(const FileDescriptor&) = delete;
    auto operator=(const FileDescriptor&) -> FileDescriptor& = delete;

    FileDescriptor(FileDescriptor&& other) noexcept
        : fd_(other.fd_)
    {
        other.fd_ = -1;
    }

    auto operator=(FileDescriptor&& other) noexcept -> FileDescriptor&
    {
        if (this != &other) {
            if (isValid()) {
                ::close(fd_);
            }
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    [[nodiscard]] auto isValid() const -> bool { return fd_ != -1; }
    [[nodiscard]] auto get() const -> int { return fd_; }

private:
    int fd_;
};