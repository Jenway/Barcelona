#pragma once
#include "http/interfaces/ICgiRunner.hpp"
#include <expected>
#include <filesystem>
#include <string>

namespace http {

class DefaultCgiRunner : public ICgiRunner {
public:
    ~DefaultCgiRunner() override = default;
    auto run(const Request& request,
        const std::filesystem::path& script_path,
        const std::string& interpreter_path) -> std::expected<std::string, std::error_code> override;
};

} // namespace http
