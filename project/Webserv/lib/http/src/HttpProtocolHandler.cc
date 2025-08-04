// lib/http/src/HttpProtocolHandler.cc
#include "HttpProtocolHandler.hpp"
#include "Error.hpp"
#include "ErrorCode.hpp"
#include "ISinker.hpp"
#include "Message.hpp"
#include <expected>
#include <fmt/format.h>

namespace http {

HttpProtocolHandler::HttpProtocolHandler(
    std::unique_ptr<IRequestParser> parser,
    std::unique_ptr<IResponseWriter> writer,
    std::unique_ptr<IRequestHandler> request_handler)
    : _parser(std::move(parser))
    , _request_handler(std::move(request_handler))
    , _response_writer(std::move(writer))
{
}

auto HttpProtocolHandler::getStatus() const -> core::protocol::Status
{
    switch (_state) {
    case State::WaitingForHeaders:
    case State::ReadingBody:
        return core::protocol::Status::WantRead;
    case State::SendingResponse:
        return core::protocol::Status::WantWrite;
    case State::Finished:
        return core::protocol::Status::Finished;
    case State::GeneratingResponse:
        break;
    }
    return core::protocol::Status::Error;
}

void HttpProtocolHandler::onData(std::string_view data)
{
    if (_state != State::WaitingForHeaders && _state != State::ReadingBody) {
        return;
    }

    const auto parse_state = _parser->parse(data);
    if (parse_state) {
        switch (*parse_state) {

        case IRequestParser::State::Parsing:
            break;
        case IRequestParser::State::Completed:
            generateResponse();
            break;
        }

    } else {
        generateResponse(true /* is_error */);
    }
}

void HttpProtocolHandler::onReadEOF()
{
    _state = State::Finished;
}

auto HttpProtocolHandler::onWriteReady(ISinker& sinker) -> std::expected<core::WriteResult, std::error_code>
{
    if (_state == State::SendingResponse && _response_writer) {
        auto res = _response_writer->writeTo(sinker);
        if (res && res->status == core::WriteResult::Status::Finished) {
            onResponseFinished();
        }
        return res;
    }
    return std::unexpected(error::to_unexpected_code(ErrorCode::Unknown));
}

void HttpProtocolHandler::onResponseFinished()
{
    const bool keep_alive = _response_writer ? _response_writer->isKeepAlive() : false;

    if (keep_alive) {
        resetForNewRequest();
    } else {
        _state = State::Finished;
    }
}

void HttpProtocolHandler::resetForNewRequest()
{
    _state = State::WaitingForHeaders;
    _parser->reset();
    _response_writer->reset();
}

void HttpProtocolHandler::generateResponse(bool is_parser_error)
{
    if (is_parser_error) {
        // 解析器错误是明确的，直接让 handler 生成一个错误响应 (e.g. 400)
        // 这个路径我们信任它不会失败
        Response response = _request_handler->handleError();
        _response_writer->bind_to(std::move(response));
        _state = State::SendingResponse;
        return;
    }

    // 从业务逻辑层获取响应，它现在返回一个 expected 对象
    auto response_or_error = _request_handler->handleRequest(_parser->getRequest());

    if (response_or_error) {
        _response_writer->bind_to(std::move(*response_or_error));

        _state = State::SendingResponse;
    } else {
        // --- 失败路径 ---
        // response_or_error 包含一个 error_code
        // 记录具体的错误原因
        // log_error("Business logic failed: {}", response_or_error.error().message());

        // 调用我们的“最后一道防线”来生成 500 错误
        generateInternalErrorResponse();
    }
}

// void HttpProtocolHandler::onHeadersCompleted()
// {
//     const auto& method = _parser->getRequest().method;

//     if (method == "GET" || method == "HEAD") {
//         generateResponse();
//     } else {
//         _state = State::ReadingBody;
//     }
// }

void HttpProtocolHandler::generateInternalErrorResponse()
{
    // 创建一个硬编码的 500 响应
    http::Response response;
    response.version = "HTTP/1.1";
    response.status_code = 500;
    response.reason_phrase = "Internal Server Error";

    // 关键：在服务器内部错误后，我们必须强制关闭连接，
    // 因为服务器状态可能已不一致。
    response.headers["Connection"] = "close";

    // 为了调试和标准，可以提供一个最小的body
    std::string body_str = "500 Internal Server Error";
    response.headers["Content-Type"] = "text/plain";
    response.headers["Content-Length"] = std::to_string(body_str.size());
    response.body = std::vector<char>(body_str.begin(), body_str.end());

    // 使用这个“安全”的响应来创建 writer
    _response_writer->bind_to(std::move(response));
    _state = State::SendingResponse;
}
} // namespace http