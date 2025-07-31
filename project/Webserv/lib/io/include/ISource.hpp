#pragma once
#include "Status.hpp"
#include <expected>
#include <system_error>
#include <vector>

/**
 * @class ISource
 * @brief 一个纯虚基类，定义了“数据源”的接口，负责执行底层的读 I/O 操作。
 */
class ISource {
public:
    virtual ~ISource() = default;

    /**
     * @brief 尝试从数据源读取数据，并将其存入提供的缓冲区。
     * @param buffer 一个用于存放读取数据的缓冲区。
     * @return 一个包含 ReadResult 和实际读取字节数的 expected，或者一个 error_code。
     */
    virtual auto read(std::vector<char>& buffer)
        -> std::expected<std::pair<core::ReadStatus, size_t>, std::error_code>
        = 0;

protected:
    ISource() = default;
    ISource(const ISource&) = delete;
    auto operator=(const ISource&) -> ISource& = delete;
    ISource(ISource&&) = delete;
    auto operator=(ISource&&) -> ISource& = delete;
};