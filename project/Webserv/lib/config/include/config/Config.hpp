#pragma once

#include <cstdint>
#include <expected>
#include <map>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

// 对应 location /old { return 301 /; }
struct ReturnDirective {
    int code;
    std::string url;
};

// 对应 location 块
struct LocationConfig {
    std::string path;

    // 可选配置项，如果 location 中没有定义，就会是 std::nullopt
    std::optional<std::string> alias;
    std::optional<std::vector<std::string>> methods;
    std::optional<std::string> index;
    std::optional<bool> autoindex;
    std::optional<size_t> client_max_body_size;
    std::optional<std::string> cgi_pass;
    std::optional<ReturnDirective> return_directive;
};

// 对应 server 块
struct ServerConfig {
    uint16_t listen;
    std::string server_name;

    // Server 级别的配置项，location 中可以覆盖
    std::string root;
    size_t client_max_body_size;

    // 映射: 错误码 -> 页面路径
    std::map<int, std::string> error_pages;

    std::vector<LocationConfig> locations;
};

// 对应整个配置文件
struct Config {
    std::vector<ServerConfig> servers;
};

auto parse_body_size(std::string_view s) -> std::expected<size_t, std::system_error>;
auto parse_config(const nlohmann::json& json) -> std::expected<Config, std::system_error>;
std::expected<ServerConfig, std::system_error> parse_server(const nlohmann::json& j);
std::expected<LocationConfig, std::system_error> parse_location(const nlohmann::json& j);
