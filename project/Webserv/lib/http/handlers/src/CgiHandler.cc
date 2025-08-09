// in lib/http/handlers/src/CgiHandler.cc
#include "http/handlers/CgiHandler.hpp"
#include "http/common/HttpStatus.hpp"
#include "http/utils/ResponseFactory.hpp"
#include "logger.hpp"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <magic_enum/magic_enum.hpp>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace http {

CgiHandler::CgiHandler(std::filesystem::path script_root,
    std::string interpreter_path,
    std::shared_ptr<ICgiRunner> runner)
    : _script_root(std::move(script_root))
    , _interpreter_path(std::move(interpreter_path))
    , _runner(std::move(runner))
{
}
auto CgiHandler::handleRequest(const Request& request) -> std::expected<Response, std::error_code>
{
    // --- 步骤 1: 确定并验证脚本路径 ---
    auto script_path_or_error = utils::resolveSafePath(_script_root, request.uri);
    if (!script_path_or_error) {
        return responses::createStockResponse<StatusCode::NotFound>();
    }

    // --- 步骤 2: 委托 Runner 执行脚本 ---
    auto result = _runner->run(request, *script_path_or_error, _interpreter_path);
    if (!result) {
        LOG_ERROR("CGI script execution failed: {}", result.error().message());
        return responses::createStockResponse<StatusCode::BadGateway>();
    }
    LOG_TRACE("Result : {}", *result);

    // --- 步骤 3: 委托辅助函数解析输出并构建响应 ---
    return parseCgiOutput(*result);
}

// **parseCgiOutput 只负责分离头和体，并协调构建 Response**
auto CgiHandler::parseCgiOutput(const std::string& raw_output) -> std::expected<Response, std::error_code>
{
    // --- 1. 寻找分隔符 ---
    const std::string_view crlf_delimiter = "\r\n\r\n";
    const std::string_view lf_delimiter = "\n\n";

    auto header_end_pos = raw_output.find(crlf_delimiter);
    size_t delimiter_len = crlf_delimiter.length();

    if (header_end_pos == std::string::npos) {
        header_end_pos = raw_output.find(lf_delimiter);
        delimiter_len = lf_delimiter.length();
    }

    if (header_end_pos == std::string::npos) {
        LOG_ERROR("CGI output missing header-body delimiter.");
        return responses::createStockResponse<StatusCode::BadGateway>();
    }

    // --- 2. 分离头和体 ---
    std::string_view header_part(raw_output.data(), header_end_pos);
    std::string_view body_part(raw_output.data() + header_end_pos + delimiter_len,
        raw_output.size() - header_end_pos - delimiter_len);

    // --- 3. 构建 Response 对象 ---
    Response response;
    response.status_code = 200; // 默认

    // --- 4. 委托 parseCgiHeaders 解析头部 ---
    parseCgiHeaders(response, header_part);

    // --- 5. 设置 Body 和 Content-Length ---
    response.headers["Content-Length"] = std::to_string(body_part.length());
    response.body = std::vector<char>(body_part.begin(), body_part.end());

    return response;
}

// **parseCgiHeaders 只负责解析头部字符串**
void CgiHandler::parseCgiHeaders(Response& response, std::string_view header_part)
{
    std::istringstream header_stream((std::string(header_part)));
    std::string line;

    while (std::getline(header_stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }

        if (std::string_view(line).starts_with("Status:")) {
            try {
                response.status_code = std::stoi(line.substr(line.find(' ') + 1));
            } catch (const std::exception&) {
                // 解析失败，忽略
            }
        } else if (auto sep_pos = line.find(": "); sep_pos != std::string::npos) {
            std::string key = line.substr(0, sep_pos);
            std::string value = line.substr(sep_pos + 2);
            response.headers[key] = value;
        }
    }
}

} // namespace http