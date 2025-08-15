## 42 Webserv

Webserv 是 [42 School](https://www.42.fr/) Common Core 中的一个 Level 05 的项目，通常是许多学生在毕业前面临的倒数第二个项目之一。项目要求使用 **C++98** 从零开始，实现一个功能完备的 HTTP/1.1 服务器。

> "This is when you finally understand why a URL starts with HTTP"

当然，我不是 42 学校的学生，所以，我“任性地”选择了**拥抱现代 C++**。本项目完全基于 **C++23** 构建，并大量采用了 `{fmt}`, `magic_enum`, `nlohmann/json` 和 `GoogleTest` 等库。

一般来说 42 学校里这个都是组队作业，某种程度上我也是组队：**Gemini, ChatGPT, Grok, Qwen, DeepSeek...** 

现在是 2025 年的 8 月 16 日，做这个做了有半个月多，还是很开心的

### **构建与尝试**

**环境要求**:
*   支持 C++23 的编译器
*   CMake (>= 3.23)
*   [vcpkg](https://vcpkg.io/en/index.html) (用于管理第三方依赖)
*   [uv](https://github.com/astral-sh/uv) (用于运行 Python E2E 测试)

**一键构建与测试:**

```bash
# 使用 CMake Presets 进行配置、构建和单元测试
cmake --workflow --preset debug
```

**运行服务器:**

```bash
# 格式: ./WebServer [config_path] [log_level: TRACE, DEBUG, INFO, WARN, ERROR]
./build/debug/WebServer config.json INFO
```

**运行端到端 (E2E) 测试:**
```bash
cd e2e_tests
uv sync # 首次运行或依赖变更时
uv run pytest
```

---

### **快速体验**

**访问静态页面:**
```bash
curl --noproxy '*' -v http://127.0.0.1:8080/
```

**体验动态 CGI (留言板):**

在浏览器中打开 `http://localhost:8080/guestbook.py`，尝试留言并刷新。

**上传并删除文件:**

```bash
# 上传
curl -X POST --data "hello upload" http://localhost:8080/upload/hello.txt

# 删除
curl -X DELETE http://localhost:8080/upload/hello.txt
```

> 当然了，如果真要生产用，还是用 NGINX 或者 Caddy 吧，哈哈。

### **Misc**

*   **详细开发日志与架构文档**: **[dev_log.md](./doc/dev_log.md)**
*   **LICENSE**: MIT License
*   **Copyright**: © 2025 Jenway
