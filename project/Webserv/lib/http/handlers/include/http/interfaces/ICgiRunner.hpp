// lib/http/interfaces/ICgiRunner.hpp
#pragma once
#include "http/common/Message.hpp"
#include <expected>
#include <filesystem>
#include <string>

namespace http {

class ICgiRunner {
public:
    virtual ~ICgiRunner() = default;

    virtual auto run(const Request& request,
        const std::filesystem::path& script_path,
        const std::string& interpreter_path) -> std::expected<std::string, std::error_code>
        = 0;
};

} // namespace http
