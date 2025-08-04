// lib/http/src/HttpProtocolHandler.cc
#include "HttpProtocolHandler.hpp"
#include "Error.hpp"
#include "ErrorCode.hpp"
#include "HttpStatus.hpp"
#include "ISinker.hpp"
#include "Message.hpp"
#include "ResponseFactory.hpp"
#include "logger.hpp"
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
        generateResponse(parse_state.error());
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

void HttpProtocolHandler::generateResponse(StatusCode code)
{
    Response response = _request_handler->handleError(code);
    _response_writer->bind_to(std::move(response));
    _state = State::SendingResponse;
}

void HttpProtocolHandler::generateResponse()
{
    // 从业务逻辑层获取响应
    auto response_or_error = _request_handler->handleRequest(_parser->getRequest());

    if (response_or_error) {
        // 业务逻辑成功，绑定正常的响应
        _response_writer->bind_to(std::move(*response_or_error));
        _state = State::SendingResponse;
    } else {
        // 业务逻辑失败了！这是一个服务器内部错误。
        // 记录错误，并生成一个标准的 500 响应。
        LOG_ERROR("Request handler failed: {}", response_or_error.error().message());
        _response_writer->bind_to(
            http::responses::createStockResponse<http::StatusCode::InternalServerError>());
        _state = State::SendingResponse;
    }
}

} // namespace http