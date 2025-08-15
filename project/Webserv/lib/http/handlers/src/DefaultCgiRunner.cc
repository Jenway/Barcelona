// lib/http/handlers/DefaultCgiRunner.cpp
#include "http/handlers/DefaultCgiRunner.hpp"
#include "FileDescriptor.hpp"
#include "logger.hpp"
#include <algorithm>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace http {

static auto createCgiEnvironment(const Request& request, const std::filesystem::path& script_path) -> std::vector<std::string>
{
    std::vector<std::string> env;

    env.emplace_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("REQUEST_METHOD=" + std::string(magic_enum::enum_name(request.method)));
    env.push_back("SERVER_PROTOCOL=" + request.version);
    env.push_back("SCRIPT_FILENAME=" + script_path.string());
    env.push_back("SCRIPT_NAME=" + request.uri);
    env.emplace_back("SERVER_NAME=localhost");
    env.emplace_back("SERVER_PORT=80");

    // Query string
    auto query_pos = request.uri.find('?');
    if (query_pos != std::string::npos) {
        env.push_back("QUERY_STRING=" + request.uri.substr(query_pos + 1));
    } else {
        env.emplace_back("QUERY_STRING=");
    }

    if (!request.body.empty()) {
        env.push_back("CONTENT_LENGTH=" + std::to_string(request.body.size()));
    }

    if (auto it = request.headers.find("Content-Type"); it != request.headers.end()) {
        env.push_back("CONTENT_TYPE=" + it->second);
    }

    for (const auto& [key, value] : request.headers) {
        std::string upper_key = key;
        std::ranges::transform(upper_key, upper_key.begin(), [](unsigned char c) {
            return c == '-' ? '_' : std::toupper(c);
        });
        env.push_back("HTTP_" + upper_key + "=" + value);
    }

    return env;
}

auto DefaultCgiRunner::run(const Request& request,
    const std::filesystem::path& script_path,
    const std::string& interpreter_path)
    -> std::expected<std::string, std::error_code>
{
    int stdin_pipe[2];
    int stdout_pipe[2];
    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }

    utils::FileDescriptor stdin_r(stdin_pipe[0]);
    utils::FileDescriptor stdin_w(stdin_pipe[1]);
    utils::FileDescriptor stdout_r(stdout_pipe[0]);
    utils::FileDescriptor stdout_w(stdout_pipe[1]);

    pid_t pid = fork();
    if (pid == -1) {
        return std::unexpected(std::make_error_code(std::errc::resource_unavailable_try_again));
    }

    if (pid == 0) {
        dup2(stdin_r.get(), STDIN_FILENO);
        dup2(stdout_w.get(), STDOUT_FILENO);
        auto script_dir = script_path.parent_path();
        if (chdir(script_dir.c_str()) == -1) {
            // 如果切换目录失败，这是一个致命错误，直接退出
            std::cerr << "CGI Error: Failed to change directory to " << script_dir << std::endl;
            _exit(127); // 127 通常表示 command not found / setup error
        }

        stdin_r.close();
        stdin_w.close();
        stdout_r.close();
        stdout_w.close();

        auto env = createCgiEnvironment(request, script_path);
        std::vector<char*> argv = {
            const_cast<char*>(interpreter_path.c_str()),
            const_cast<char*>(script_path.c_str()),
            nullptr
        };
        std::vector<char*> envp;
        envp.reserve(env.size());
        for (const auto& s : env)
            envp.push_back(const_cast<char*>(s.c_str()));
        envp.push_back(nullptr);

        execve(argv[0], argv.data(), envp.data());
        std::cerr << "Execve failed!\n";
        _exit(1);
    }

    // 父进程
    stdin_r.close();
    stdout_w.close();

    if (!request.body.empty()) {
        write(stdin_w.get(), request.body.data(), request.body.size());
    }
    stdin_w.close();

    std::string output;
    char buffer[4096];
    ssize_t n = 0;
    while ((n = read(stdout_r.get(), buffer, sizeof(buffer))) > 0) {
        output.append(buffer, n);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return std::unexpected(std::make_error_code(std::errc::bad_message));
    }

    return output;
}

}
