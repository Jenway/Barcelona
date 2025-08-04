// in lib/http/include/StaticFileHandler.hpp
#pragma once

#include "http/interfaces/IRequestHandler.hpp"
#include <filesystem>

namespace http {

class StaticFileHandler : public IRequestHandler {
public:
    explicit StaticFileHandler(std::filesystem::path root_path);
    auto handleRequest(const Request& request)
        -> std::expected<Response, std::error_code> override;

private:
    const std::filesystem::path _root_path;
};

} // namespace http