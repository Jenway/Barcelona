// in lib/http/include/Serializer.hpp
#pragma once

#include "http/common/Message.hpp"
#include <fmt/core.h>
#include <string>

namespace http {

// Serializes ONLY the headers of a Response object.
inline std::string serialize_headers(const http::Response& response)
{
    std::string headers;
    headers.reserve(256); // Pre-allocate
    fmt::format_to(
        std::back_inserter(headers),
        "{} {} {}\r\n",
        response.version,
        response.status_code,
        response.reason_phrase);
    for (const auto& [key, value] : response.headers) {
        fmt::format_to(std::back_inserter(headers), "{}: {}\r\n", key, value);
    }
    headers += "\r\n";
    return headers;
}

} // namespace http