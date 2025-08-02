// in main.cc

#include "Server.hpp"
#include "logger.hpp"
#include <unistd.h>

int main()
{
    if (isatty(STDOUT_FILENO) == 0) {
        Logger::enableColor(false);
    }
    Logger::setRuntimeLogLevel(Logger::LogLevel::TRACE);

    try {
        Server server(8080); // 监听 8080 端口

        LOG_INFO("Setting up server...");
        server.setup();

        LOG_INFO("Server starting to run on port 8080...");
        server.run();

        LOG_INFO("Server has shut down.");

    } catch (const std::system_error& e) {
        LOG_ERROR("A critical error occurred during server startup: {}", e);
        return 1;
    } catch (const std::exception& e) {
        LOG_ERROR("An unexpected exception occurred: {}", e.what());
        return 1;
    }

    return 0;
}