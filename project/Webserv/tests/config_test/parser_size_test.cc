#include "config/Config.hpp"
#include <fmt/format.h>
#include <gtest/gtest.h>

using namespace std;

TEST(ParseBodySizeTest, ValidInputs)
{
    EXPECT_EQ(parse_body_size("10MB").value(), 10 * 1024 * 1024);
    EXPECT_EQ(parse_body_size("1kb").value(), 1024);
    EXPECT_EQ(parse_body_size("500").value(), 500); // 默认为字节
    EXPECT_EQ(parse_body_size("2 GB").value(), 2ULL * 1024 * 1024 * 1024);
    EXPECT_EQ(parse_body_size("1.5MB").value(), static_cast<size_t>(1.5 * 1024 * 1024));
    EXPECT_EQ(parse_body_size("  100  KB ").value(), 100 * 1024);
}

TEST(ParseBodySizeTest, InvalidInputs)
{
    std::vector<std::string_view> invalid_inputs = {
        "", "abc", "12XY", "5MBB", "1.8.2KB"
    };

    for (auto input : invalid_inputs) {
        SCOPED_TRACE(fmt::format("Testing parse_body_size(\"{}\")", input));
        auto result = parse_body_size(input);
        if (result.has_value()) {
            std::cerr << "Unexpected success: value = " << *result << " for input = " << input << "\n";
        }
        EXPECT_FALSE(result.has_value());
    }
}

TEST(ParseBodySizeTest, EdgeCases)
{
    // 超大值测试
    auto res = parse_body_size("999999999999GB");
    EXPECT_FALSE(res.has_value());

    // 小数位过多也会失败（因为溢出）
    EXPECT_FALSE(parse_body_size("1e1000GB").has_value());
}
