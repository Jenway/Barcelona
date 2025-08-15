// in main.cc

#include "Server.hpp"
#include "config/Config.hpp"
#include "logger.hpp"
#include <expected>
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <unistd.h>

struct AppSettings {
    std::string configPath = "config.json";
    Logger::LogLevel logLevel = Logger::LogLevel::INFO;
};

namespace {

auto parseCommandLine(int argc, char* argv[]) -> std::expected<AppSettings, std::string>
{
    AppSettings settings;
    if (argc >= 2) {
        settings.configPath = argv[1];
    }
    if (argc >= 3) {
        const auto result = Logger::stringToLevel(argv[2]);
        if (!result) {
            return std::unexpected("Invalid log level provided.");
        }
        settings.logLevel = *result;
    }
    return settings;
}

auto loadConfiguration(const std::string& path) -> std::expected<Config, std::string>
{
    std::ifstream configFile(path);
    if (!configFile.is_open()) {
        return std::unexpected("Failed to open configuration file: " + path);
    }

    nlohmann::json jsonData;
    try {
        jsonData = nlohmann::json::parse(configFile);
    } catch (const nlohmann::json::parse_error& e) {
        return std::unexpected("Failed to parse JSON configuration: " + std::string(e.what()));
    }

    auto config_or_error = parse_config(jsonData);
    if (!config_or_error) {
        return std::unexpected("Failed to validate config semantics: " + std::string(config_or_error.error().what()));
    }

    return *config_or_error;
}
} // namespace

int main(int argc, char* argv[])
{
    // 步骤 1: 解析命令行参数
    auto settings_result = parseCommandLine(argc, argv);
    if (!settings_result) {
        LOG_ERROR("{}", settings_result.error());
        LOG_INFO("Usage: {} [config_path] [log_level]", argv[0]);
        return 1;
    }
    const auto& settings = *settings_result;

    // 步骤 2: 初始化日志系统
    if (isatty(STDOUT_FILENO) == 0) {
        Logger::enableColor(false);
    }
    Logger::setRuntimeLogLevel(settings.logLevel);

    LOG_INFO("Using config: {}", settings.configPath);
    LOG_INFO("Log level set to: {}", settings.logLevel);

    // 步骤 3: 加载配置
    auto config_result = loadConfiguration(settings.configPath);
    if (!config_result) {
        LOG_ERROR("Configuration error: {}", config_result.error());
        return 1;
    }
    const auto& config = *config_result;

    // 步骤 4: 创建并运行服务器
    LOG_INFO("Setting up server...");
    auto server_result = Server::create(config);
    if (!server_result) {
        LOG_ERROR("Failed to create server: {}", server_result.error());
        return 1;
    }

    (*server_result)->run();

    LOG_INFO("Server shut down gracefully. Goodbye.");
    return 0;
}