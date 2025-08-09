#include "http/handlers/DefaultCgiRunner.hpp"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

using namespace http;

class CgiRunnerTest : public ::testing::Test {
protected:
    std::filesystem::path script_path;
    std::string interpreter;

    void SetUp() override
    {
        // 临时文件
        script_path = std::filesystem::temp_directory_path() / "test_cgi_script.py";

        // 创建一个简单的 Python CGI 脚本
        std::ofstream script(script_path);
        script << "#!/usr/bin/env python3\n"
                  "print(\"Content-Type: text/plain\")\n"
                  "print()\n"
                  "print(\"Hello from CGI\")\n";
        script.close();

        // 设置可执行权限
        std::filesystem::permissions(script_path,
            std::filesystem::perms::owner_exec | std::filesystem::perms::owner_read | std::filesystem::perms::group_exec | std::filesystem::perms::group_read | std::filesystem::perms::others_exec | std::filesystem::perms::others_read,
            std::filesystem::perm_options::add);

        interpreter = "/usr/bin/python3"; // 或根据系统设定修改
    }

    void TearDown() override
    {
        std::filesystem::remove(script_path);
    }
};

TEST_F(CgiRunnerTest, RunsSimplePythonScript)
{
    DefaultCgiRunner runner;

    Request req;
    req.method = Method::GET;
    req.uri = "/test_cgi_script.py";
    req.version = "HTTP/1.1";

    auto result = runner.run(req, script_path, interpreter);
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("Hello from CGI"), std::string::npos);
}
