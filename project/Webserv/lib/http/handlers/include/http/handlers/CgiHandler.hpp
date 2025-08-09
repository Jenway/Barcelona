// in lib/http/handlers/include/http/handlers/CgiHandler.hpp
#pragma once
#include "http/interfaces/ICgiRunner.hpp"
#include "http/interfaces/IRequestHandler.hpp"
#include <filesystem>

namespace http {
class CgiHandler : public IRequestHandler {
public:
    CgiHandler(std::filesystem::path script_root,
        std::string interpreter_path,
        std::shared_ptr<ICgiRunner> runner);

    auto handleRequest(const Request& request) -> std::expected<Response, std::error_code> override;

private:
    auto parseCgiOutput(const std::string& raw_output)
        -> std::expected<Response, std::error_code>;

    void parseCgiHeaders(Response& response, std::string_view header_part);

    std::filesystem::path _script_root;
    std::string _interpreter_path;
    std::shared_ptr<ICgiRunner> _runner;
};

} // namespace http