// lib/http/src/HttpProtocolHandler.cc
#include "http/core/HttpProtocolHandler.hpp"
#include "Error.hpp"
#include "ErrorCode.hpp"
#include "ISinker.hpp"
#include "http/common/HttpStatus.hpp"
#include "http/common/Message.hpp"
#include "http/utils/ResponseFactory.hpp"
#include "logger.hpp"
#include <expected>
#include <fmt/format.h>

namespace http {

HttpProtocolHandler::HttpProtocolHandler(
    std::unique_ptr<IRequestParser> parser,
    std::unique_ptr<IResponseWriter> writer,
    std::shared_ptr<IRequestDispatcher> request_router)
    : _parser(std::move(parser))
    , _request_router(std::move(request_router))
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
        LOG_WARN("Error parsing ： {}", parse_state.error());
        generateResponse(parse_state.error());
    }
}

void HttpProtocolHandler::onReadEOF()
{
    LOG_INFO("Recieved Client EOF,But we are not going to do anything :)");
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

void HttpProtocolHandler::processResponse(Response response)
{
    const bool client_wants_keep_alive = _parser->clientWantsKeepAlive();
    const bool server_allows_keep_alive = (response.status_code < 400);
    const bool final_keep_alive = client_wants_keep_alive && server_allows_keep_alive;

    if (final_keep_alive) {
        response.headers["Connection"] = "keep-alive";
    } else {
        response.headers["Connection"] = "close";
    }

    _response_writer->setKeepAlive(final_keep_alive);
    LOG_INFO("Processing response: {}", response);

    _response_writer->bind_to(std::move(response));
    _state = State::SendingResponse;
}

void HttpProtocolHandler::generateResponse(StatusCode code)
{
    Response response = _request_router->handleError(code);
    processResponse(std::move(response));
}

void HttpProtocolHandler::generateResponse()
{
    auto req = _parser->getRequest();
    auto response_or_error = _request_router->handleRequest(req);
    LOG_INFO("Handling request: {}", req);
    if (response_or_error) {
        processResponse(std::move(*response_or_error));
    } else {
        LOG_ERROR("Request handler failed: {}", response_or_error.error().message());
        processResponse(responses::createStockResponse<StatusCode::InternalServerError>());
    }
}

} // namespace http