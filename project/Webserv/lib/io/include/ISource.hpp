#pragma once
#include "Status.hpp"
#include <expected>
#include <system_error>
#include <vector>

class ISource {
public:
    virtual ~ISource() = default;
    virtual auto read(std::vector<char>& buffer)
        -> std::expected<core::ReadResult, std::error_code>
        = 0;

    ISource() = default;
    ISource(const ISource&) = delete;
    auto operator=(const ISource&) -> ISource& = delete;
    ISource(ISource&&) = delete;
    auto operator=(ISource&&) -> ISource& = delete;
};