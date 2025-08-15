// in lib/http/include/Message.hpp
#pragma once

#include "Status.hpp"
#include <fmt/core.h>
#include <fmt/ranges.h>
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

namespace fmt {

template <>
struct formatter<http::Request> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const http::Request& req, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "{} {} {}\nHeaders: {}\nBody size: {}",
            magic_enum::enum_name(req.method),
            req.uri,
            req.version,
            fmt::join(req.headers, ", "),
            req.body.size());
    }
};

template <>
struct formatter<http::Response> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const http::Response& res, FormatContext& ctx) const
    {
        size_t body_size = 0;
        if (std::holds_alternative<std::vector<char>>(res.body)) {
            body_size = std::get<std::vector<char>>(res.body).size();
        } else if (std::holds_alternative<http::FileBody>(res.body)) {
            body_size = std::get<http::FileBody>(res.body).size;
        }

        return fmt::format_to(
            ctx.out(),
            "{} {} {}\nHeaders: {}\nBody size: {}",
            res.version,
            res.status_code,
            res.reason_phrase,
            fmt::join(res.headers, ", "),
            body_size);
    }
};

} // namespace fmt