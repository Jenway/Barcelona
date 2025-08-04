// in lib/http/include/Message.hpp
#pragma once

#include "Status.hpp"
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace http {

struct FileBody {
    int fd {};
    size_t size {};
    off_t offset = 0;
};

using ResponseBody = std::variant<
    std::vector<char>,
    FileBody>;

struct Request {
    Method method = Method::GET;
    std::string uri = "/";
    std::string version = "HTTP/1.1";
    std::map<std::string, std::string> headers;
    std::vector<char> body;
};

struct Response {
    std::string version = "HTTP/1.1";
    int status_code = 200;
    std::string reason_phrase = "OK";
    std::map<std::string, std::string> headers;
    ResponseBody body;
};

} // namespace http