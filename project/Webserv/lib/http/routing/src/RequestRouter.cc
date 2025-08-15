// in lib/http/src/RequestRouter.cc
#include "http/routing/RequestRouter.hpp"
#include "http/common/HttpStatus.hpp"
#include "http/common/Message.hpp"
#include "http/utils/ResponseFactory.hpp"

namespace http {

void RequestRouter::use(std::unique_ptr<http::IRequestHandler> handler)
{
    _pipeline.push_back(std::move(handler));
}

auto RequestRouter::handleRequest(const http::Request& request) -> std::expected<http::Response, std::error_code>
{
    // 依次尝试管道中的每一个处理器
    for (const auto& handler : _pipeline) {
        auto response_or_error = handler->handleRequest(request);

        // 如果处理器返回了一个不是 404 的响应，我们就认为它成功处理了
        if (response_or_error && response_or_error->status_code != 404) {
            return response_or_error; // 立刻返回这个成功的响应
        }
        // 如果处理器返回 404，我们就继续尝试下一个处理器
    }

    // 如果所有处理器都试过了（都返回了 404），那么最终返回 404
    return http::responses::createStockResponse<http::StatusCode::NotFound>();
}

auto RequestRouter::handleError(http::StatusCode code) -> Response
{
    return http::responses::createStockResponse(code);
}

} // namespace http