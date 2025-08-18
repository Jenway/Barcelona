#include "bvh.hpp"
#include "parser.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

// --- 测试固件，用于管理临时文件 ---
class ParserTest : public ::testing::Test {
protected:
    // 在每个测试开始前创建临时文件
    void SetUp() override
    {
        temp_filename = "test_scene.rt";
    }

    // 在每个测试结束后删除临时文件
    void TearDown() override
    {
        std::remove(temp_filename.c_str());
    }

    // 辅助函数，用于向临时文件写入内容
    void WriteToFile(const std::string& content)
    {
        std::ofstream file(temp_filename);
        file << content;
        file.close();
    }

    std::string temp_filename;
};

// ========================================================================
// 1. 测试匿名空间中的所有辅助函数
// ========================================================================

TEST(ParserHelpersTest, ConvertString)
{
    // 正常情况
    EXPECT_EQ(Parser::convert_string<int>("123"), 123);
    EXPECT_DOUBLE_EQ(Parser::convert_string<double>("-45.67"), -45.67);

    // 错误情况
    EXPECT_THROW(Parser::convert_string<int>("abc"), std::invalid_argument);
    EXPECT_THROW(Parser::convert_string<double>("12.34.56"), std::invalid_argument);
    EXPECT_THROW(Parser::convert_string<double>("12a"), std::invalid_argument); // 包含非数字
    EXPECT_THROW(Parser::convert_string<double>("12.3%"), std::invalid_argument); // 测试您提到的 0.3% 问题
}

TEST(ParserHelpersTest, Split)
{
    std::string_view s = "a,b,c";
    std::vector<std::string_view> expected = { "a", "b", "c" };
    EXPECT_EQ(Parser::split(s, ','), expected);

    s = "1.0,-2.5,3.0";
    expected = { "1.0", "-2.5", "3.0" };
    EXPECT_EQ(Parser::split(s, ','), expected);

    s = "single";
    expected = { "single" };
    EXPECT_EQ(Parser::split(s, ','), expected);

    s = "a,,b"; // 空字段
    expected = { "a", "", "b" };
    EXPECT_EQ(Parser::split(s, ','), expected);
}

TEST(ParserHelpersTest, ParseVec3)
{
    Vec3 v = Parser::parse_vec3("1.0,-2.5,3.0");
    EXPECT_DOUBLE_EQ(v.x(), 1.0);
    EXPECT_DOUBLE_EQ(v.y(), -2.5);
    EXPECT_DOUBLE_EQ(v.z(), 3.0);

    // 错误情况
    EXPECT_THROW(Parser::parse_vec3("1,2"), std::runtime_error); // 组件太少
    EXPECT_THROW(Parser::parse_vec3("1,2,3,4"), std::runtime_error); // 组件太多
    EXPECT_THROW(Parser::parse_vec3("1,a,3"), std::invalid_argument); // 无效数字
}

TEST(ParserHelpersTest, ParseColor)
{
    Vec3 c = Parser::parse_color("255,127.5,0");
    EXPECT_DOUBLE_EQ(c.x(), 1.0);
    EXPECT_DOUBLE_EQ(c.y(), 0.5);
    EXPECT_DOUBLE_EQ(c.z(), 0.0);

    // 错误情况
    EXPECT_THROW(Parser::parse_color("255,128"), std::runtime_error);
    EXPECT_THROW(Parser::parse_color("255,g,0"), std::invalid_argument);
}

TEST(ParserHelpersTest, ParseSpecularParams)
{
    auto [color, shininess] = Parser::parse_specular_params("0.8,128");
    EXPECT_DOUBLE_EQ(color.x(), 0.8);
    EXPECT_DOUBLE_EQ(color.y(), 0.8);
    EXPECT_DOUBLE_EQ(color.z(), 0.8);
    EXPECT_DOUBLE_EQ(shininess, 128.0);

    // 错误情况
    EXPECT_THROW(Parser::parse_specular_params("0.8"), std::runtime_error);
    EXPECT_THROW(Parser::parse_specular_params("0.8,128,extra"), std::runtime_error);
    EXPECT_THROW(Parser::parse_specular_params("abc,128"), std::invalid_argument);
}

// ========================================================================
// 2. 测试 FileParser 类的核心功能
// ========================================================================

// 测试一个最小的、有效的场景文件
TEST_F(ParserTest, MinimalValidScene)
{
    WriteToFile(
        "A 0.2 255,255,255\n"
        "C 0,0,0 0,0,-1 70\n");
    Scene scene = Parser::parse_file(temp_filename);

    EXPECT_TRUE(scene.has_ambient_light);
    EXPECT_DOUBLE_EQ(scene.ambient_light.ratio, 0.2);

    EXPECT_TRUE(scene.has_camera);
    EXPECT_DOUBLE_EQ(scene.camera.fov, 70.0);
    EXPECT_TRUE(scene.lights.empty());
    EXPECT_EQ(scene.world, nullptr); // 没有物体
}

// 测试平面和圆柱的解析
TEST_F(ParserTest, ParsePlaneAndCylinder)
{
    WriteToFile(
        "A 0.1 1,2,3\n"
        "C 0,0,0 0,0,-1 70\n"
        "pl 0,-1,0 0,1,0 0,255,0\n" // 只有漫反射颜色
        "cy 1,2,3 0,0,1 10 20 0,0,255 0.5,64\n" // 带高光
    );
    Parser::FileParser parser(temp_filename);
    Scene scene = parser.parse();
    ASSERT_NE(scene.world, nullptr);
    // BVH 的根节点应该存在
    auto bvh_node = std::dynamic_pointer_cast<BVHNode>(scene.world);
    ASSERT_NE(bvh_node, nullptr);
}

// 测试注释和空行被正确忽略
TEST_F(ParserTest, HandlesCommentsAndEmptyLines)
{
    WriteToFile(
        "# This is a comment\n"
        "A 0.2 255,255,255\n"
        "\n" // Empty line
        "C 0,0,0 0,0,-1 70 # Inline comment\n");
    // 如果没有抛出异常，就说明解析成功
    ASSERT_NO_THROW(Parser::parse_file(temp_filename));
    Scene scene = Parser::parse_file(temp_filename);
    EXPECT_TRUE(scene.has_camera);
    EXPECT_TRUE(scene.has_ambient_light);
}

// ========================================================================
// 3. 测试所有可预见的错误情况
// ========================================================================

TEST_F(ParserTest, ThrowsOnNonExistentFile)
{
    EXPECT_THROW(Parser::parse_file("non_existent_file.rt"), std::runtime_error);
}

TEST_F(ParserTest, ThrowsOnMissingCamera)
{
    WriteToFile("A 0.2 255,255,255\n");
    // 使用 ASSERT_THROW 验证是否抛出了特定类型的异常
    ASSERT_THROW(Parser::parse_file(temp_filename), Parser::FileParser::ParsingError);
    try {
        Parser::parse_file(temp_filename);
    } catch (const Parser::FileParser::ParsingError& e) {
        // 验证错误消息是否包含我们期望的文本
        EXPECT_NE(std::string(e.what()).find("must contain a camera"), std::string::npos);
    }
}

TEST_F(ParserTest, ThrowsOnMissingAmbientLight)
{
    WriteToFile("C 0,0,0 0,0,-1 70\n");
    ASSERT_THROW(Parser::parse_file(temp_filename), Parser::FileParser::ParsingError);
}

TEST_F(ParserTest, ThrowsOnDuplicateCamera)
{
    WriteToFile(
        "A 0.2 255,255,255\n"
        "C 0,0,0 0,0,-1 70\n"
        "C 1,1,1 0,0,-1 80\n");
    ASSERT_THROW(Parser::parse_file(temp_filename), Parser::FileParser::ParsingError);
}

TEST_F(ParserTest, ThrowsOnUnknownElementType)
{
    WriteToFile(
        "A 0.2 255,255,255\n"
        "C 0,0,0 0,0,-1 70\n"
        "UNKNOWN 1 2 3\n" // 未知类型
    );
    ASSERT_THROW(Parser::parse_file(temp_filename), Parser::FileParser::ParsingError);
}

TEST_F(ParserTest, ThrowsOnIncorrectArgumentCount)
{
    WriteToFile(
        "A 0.2 255,255,255\n"
        "C 0,0,0 0,0,-1\n" // 参数太少
    );
    ASSERT_THROW(Parser::parse_file(temp_filename), Parser::FileParser::ParsingError);
}

TEST_F(ParserTest, ThrowsOnInvalidNumericValue)
{
    WriteToFile(
        "A 0.2 255,255,255\n"
        "C 0,0,0 0,0,-1 seventy\n" // 无效FOV
    );
    ASSERT_THROW(Parser::parse_file(temp_filename), Parser::FileParser::ParsingError);
}

TEST_F(ParserTest, ThrowsOnInvalidVectorFormat)
{
    WriteToFile(
        "A 0.2 255,255,255\n"
        "C 0,0,0 0,-1 70\n" // 向量格式错误
    );
    ASSERT_THROW(Parser::parse_file(temp_filename), Parser::FileParser::ParsingError);
}

TEST_F(ParserTest, ThrowsOnInvalidFovValue)
{
    WriteToFile(
        "A 0.2 255,255,255\n"
        "C 0,0,0 0,0,-1 180\n" // FOV超出范围
    );
    ASSERT_THROW(Parser::parse_file(temp_filename), Parser::FileParser::ParsingError);
}
