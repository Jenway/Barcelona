// src/main.cc

#include "logger.hpp"
#include <string>

#include <unistd.h>

void process_data(const std::string& data)
{
    LOG_TRACE("Entering function process_data with data: '{}'", data);
    if (data.empty()) {
        LOG_WARN("Processing empty data string.");
    }
    int user_id = 123;
    LOG_INFO("Processing data for user_id: {}", user_id);
    errno = EACCES;
    LOG_ERROR("Failed to open resource: {}", Logger::SysError("accessing /data/db"));
}

int main()
{
    if (!isatty(STDOUT_FILENO)) {
        Logger::enableColor(false);
    }

    Logger::setRuntimeLogLevel(Logger::LogLevel::TRACE);
    LOG_INFO("Application starting up.");
    process_data("sample_payload");
    LOG_INFO("Application finished.");

    return 0;
}