#include "config/Config.hpp"
#include "nlohmann/json.hpp"
#include <gtest/gtest.h>

using json = nlohmann::json;

TEST(ParseConfigTest, ValidLocation)
{
    json loc = {
        { "path", "/upload" },
        { "alias", "/uploads" },
        { "methods", { "POST", "DELETE" } },
        { "client_max_body_size", "10MB" },
        { "autoindex", true }
    };

    auto result = parse_location(loc);
    ASSERT_TRUE(result.has_value());

    const auto& l = *result;
    EXPECT_EQ(l.path, "/upload");
    EXPECT_TRUE(l.alias.has_value());
    EXPECT_EQ(*l.alias, "/uploads");
    EXPECT_TRUE(l.methods.has_value());
    EXPECT_EQ((*l.methods).size(), 2);
    EXPECT_EQ(*l.client_max_body_size, 10 * 1024 * 1024);
    EXPECT_EQ(l.autoindex, true);
}

TEST(ParseConfigTest, ReturnDirective)
{
    json loc = {
        { "path", "/old" },
        { "return", { { "code", 301 }, { "url", "/" } } }
    };

    auto result = parse_location(loc);

    ASSERT_TRUE(result.has_value()) << "Parse error: " << result.error().what() << "\n";
    ASSERT_TRUE(result->return_directive.has_value()) << "return_directive is nullopt";
    EXPECT_EQ(result->return_directive->code, 301);
    EXPECT_EQ(result->return_directive->url, "/");
}

TEST(ParseConfigTest, ServerConfigValid)
{
    json server = {
        { "listen", 8080 },
        { "server_name", "localhost" },
        { "root", "/www" },
        { "client_max_body_size", "5MB" },
        { "error_pages", { { "404", "/404.html" }, { "500", "/500.html" } } },
        { "locations", json::array({ { { "path", "/" }, { "methods", { "GET" } }, { "index", "index.html" } } }) }
    };

    auto result = parse_server(server);
    if (!result) {
        std::cerr << "Parse error: " << result.error().what() << "\n";
    }
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->listen, 8080);
    EXPECT_EQ(result->server_name, "localhost");
    EXPECT_EQ(result->root, "/www");
    EXPECT_EQ(result->client_max_body_size, 5 * 1024 * 1024);
    EXPECT_EQ(result->error_pages.at(404), "/404.html");
}
