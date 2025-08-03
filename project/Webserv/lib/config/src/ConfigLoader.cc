#include "Config.hpp"
#include "Error.hpp"
#include "ErrorCode.hpp"

#include "nlohmann/json.hpp"

#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <fmt/core.h>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>

auto parse_config(const nlohmann::json& json) -> std::expected<Config, std::system_error>
{
    if (!json.contains("servers") || !json["servers"].is_array()) {
        return error::to_unexpected(std::errc::invalid_argument, "no Keyword: Server found");
    }

    Config config;
    for (const auto& server_json : json["servers"]) {
        auto server = parse_server(server_json);
        if (!server)
            return std::unexpected(server.error());
        config.servers.push_back(*std::move(server));
    }

    return config;
}

std::expected<ServerConfig, std::system_error> parse_server(const nlohmann::json& j)
{
    ServerConfig server;

    if (!j.contains("listen") || !j["listen"].is_number()) {
        return error::to_unexpected(
            std::errc::invalid_argument,
            fmt::format("ServerConfig: missing or invalid 'listen' (expected number), json: {}", j.dump(1)));
    }

    int64_t port = j["listen"].get<int64_t>();
    if (port < 0 || port > 65535) {
        return error::to_unexpected(
            std::errc::invalid_argument,
            fmt::format("ServerConfig: port out of range (0~65535), got: {}, json: {}", port, j.dump(1)));
    }
    server.listen = static_cast<uint16_t>(port);
    server.server_name = j.value("server_name", "default");

    if (!j.contains("root") || !j["root"].is_string()) {
        return error::to_unexpected(
            std::errc::invalid_argument,
            fmt::format("ServerConfig: missing or invalid 'root' (expected string), json: {}", j.dump(1)));
    }

    server.root = j["root"].get<std::string>();

    if (auto body_sz = parse_body_size(j.value("client_max_body_size", "1MB"));
        body_sz) {
        server.client_max_body_size = *body_sz;
    } else {
        return std::unexpected(body_sz.error());
    }

    if (j.contains("error_pages")) {
        for (auto it = j["error_pages"].begin(); it != j["error_pages"].end(); ++it) {
            int code = std::stoi(it.key());
            server.error_pages[code] = it.value().get<std::string>();
        }
    }

    if (j.contains("locations") && j["locations"].is_array()) {
        for (const auto& loc_json : j["locations"]) {
            auto loc = parse_location(loc_json);
            if (!loc)
                return std::unexpected(loc.error());
            server.locations.push_back(*std::move(loc));
        }
    }

    return server;
}

std::expected<LocationConfig, std::system_error> parse_location(const nlohmann::json& j)
{
    LocationConfig loc;

    if (!j.contains("path") || !j["path"].is_string()) {
        return error::to_unexpected(
            std::errc::invalid_argument,
            fmt::format("LocationConfig: missing or invalid 'path' (expected string), json: {}", j.dump(1)));
    }
    loc.path = j["path"].get<std::string>();

    if (j.contains("alias")) {
        loc.alias = j["alias"].get<std::string>();
    }

    if (j.contains("methods") && j["methods"].is_array()) {
        std::vector<std::string> methods;
        for (const auto& m : j["methods"]) {
            methods.push_back(m.get<std::string>());
        }
        loc.methods = std::move(methods);
    }

    if (j.contains("index")) {
        loc.index = j["index"].get<std::string>();
    }

    if (j.contains("autoindex")) {
        loc.autoindex = j["autoindex"].get<bool>();
    }

    if (j.contains("client_max_body_size")) {
        if (auto sz = parse_body_size(j["client_max_body_size"].get<std::string>()); sz) {
            loc.client_max_body_size = *sz;
        } else {
            return std::unexpected(sz.error());
        }
    }

    if (j.contains("cgi_pass")) {
        loc.cgi_pass = j["cgi_pass"].get<std::string>();
    }

    if (j.contains("return") && j["return"].is_object()) {
        const auto& r = j["return"];
        if (!(r.contains("code") && r.contains("url"))) {
            return error::to_unexpected(
                std::errc::invalid_argument,
                fmt::format("LocationConfig: incomplete 'return' directive (missing 'code' or 'url'), json: {}", r.dump()));
        }
        ReturnDirective redirect;
        redirect.code = r["code"].get<int>();
        redirect.url = r["url"].get<std::string>();
        loc.return_directive = redirect;
    }

    return loc;
}
auto parse_body_size(std::string_view s) -> std::expected<size_t, std::system_error>
{
    // --- 1. trim leading whitespace ---
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])))
        ++i;
    if (i == s.size()) {
        return error::to_unexpected(
            ErrorCode::Config_InvalidBodySize,
            "Input is empty or whitespace only");
    }
    s.remove_prefix(i);

    // --- 2. scan numeric part (allow at most one '.') ---
    size_t num_start = 0;
    size_t num_end = 0;
    int dot_count = 0;
    while (num_end < s.size()) {
        char c = s[num_end];
        if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
            ++num_end;
        } else if (c == '.') {
            if (++dot_count > 1) {
                return error::to_unexpected(
                    ErrorCode::Config_InvalidBodySize,
                    fmt::format("Invalid numeric format (multiple dots) in '{}'", s));
            }
            ++num_end;
        } else {
            break;
        }
    }
    if (num_end == num_start) {
        return error::to_unexpected(
            ErrorCode::Config_InvalidBodySize,
            fmt::format("Missing numeric value in '{}'", s));
    }
    auto number_str = std::string(s.substr(num_start, num_end - num_start));

    // --- 3. skip whitespace before unit ---
    size_t u = num_end;
    while (u < s.size() && (std::isspace(static_cast<unsigned char>(s[u])) != 0))
        ++u;
    auto unit_sv = s.substr(u);

    // --- 4. normalize unit to uppercase string ---
    std::string unit;
    unit.reserve(unit_sv.size());
    for (char c : unit_sv) {
        if (std::isalpha(static_cast<unsigned char>(c)) != 0) {
            unit += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        } else if (std::isspace(static_cast<unsigned char>(c)) == 0) {
            return error::to_unexpected(
                ErrorCode::Config_InvalidBodySize,
                fmt::format("Unexpected character '{}' in unit of '{}'", c, s));
        }
    }

    // --- 5. parse the numeric value ---
    long double value = 0.0L;
    try {
        value = std::stold(number_str);
    } catch (...) {
        return error::to_unexpected(
            ErrorCode::Config_InvalidBodySize,
            fmt::format("Invalid numeric value '{}'", number_str));
    }

    // --- 6. lookup multiplier ---
    static constexpr std::pair<std::string_view, uint64_t> table[] = {
        { "", 1ULL },
        { "B", 1ULL },
        { "K", 1024ULL },
        { "KB", 1024ULL },
        { "M", 1024ULL * 1024ULL },
        { "MB", 1024ULL * 1024ULL },
        { "G", 1024ULL * 1024ULL * 1024ULL },
        { "GB", 1024ULL * 1024ULL * 1024ULL },
    };

    uint64_t mul = 0;
    for (const auto& kv : table) {
        if (unit == kv.first) {
            mul = kv.second;
            break;
        }
    }
    if (mul == 0) {
        return error::to_unexpected(
            ErrorCode::Config_InvalidBodySize,
            fmt::format("Unknown unit '{}' in '{}'", unit, s));
    }

    // --- 7. compute and range-check ---
    long double result = value * static_cast<long double>(mul);
    if (result < 0 || result > static_cast<long double>(std::numeric_limits<size_t>::max())) {
        return error::to_unexpected(
            ErrorCode::Config_InvalidBodySize,
            fmt::format("Value '{}' out of range", s));
    }

    return static_cast<size_t>(std::round(result));
}
