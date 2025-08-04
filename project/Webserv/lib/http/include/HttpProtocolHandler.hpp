// in lib/http/include/HttpProtocolHandler.hpp
#pragma once

#include "HttpStatus.hpp"
#include "IProtocolHandler.hpp"
#include "IRequestHandler.hpp"
#include "IRequestParser.hpp"
#include "IResponseWriter.hpp"
#include <cstdint>
#include <memory>

namespace http {

class HttpProtocolHandler : public protocol::IHandler {
public:
    explicit HttpProtocolHandler(
        std::unique_ptr<IRequestParser> parser,
        std::unique_ptr<IResponseWriter> writer,
        std::unique_ptr<IRequestHandler> request_handler);
    ~HttpProtocolHandler() override = default;

    // --- IHandler 接口实现 ---
    void onData(std::string_view data) override;
    void onReadEOF() override;
    auto onWriteReady(ISinker& sinker) -> std::expected<core::WriteResult, std::error_code> override;
    [[nodiscard]] auto getStatus() const -> core::protocol::Status override;

private:
    enum class State : uint8_t {
        WaitingForHeaders,
        ReadingBody,
        GeneratingResponse,
        SendingResponse,
        Finished
    };
    void generateResponse();
    void generateResponse(StatusCode code);
    void onResponseFinished();
    void resetForNewRequest();

    State _state = State::WaitingForHeaders;

    std::unique_ptr<IRequestParser> _parser;
    std::unique_ptr<IRequestHandler> _request_handler;
    std::unique_ptr<IResponseWriter> _response_writer;

    std::string _serialized_headers;
    size_t _header_bytes_sent = 0;
    size_t _body_bytes_sent = 0;
};

} // namespace http