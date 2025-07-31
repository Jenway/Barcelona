#pragma once

#include <memory>

/**
 * @class IResponse
 * @brief 一个纯虚基类，代表任何可以被 ISinker 发送的“响应”对象。
 *
 * 它本身不包含任何数据或方法，仅作为一个类型标记，
 * 允许 Connection 和 ISinker 以一种通用的方式处理不同类型的响应
 * (例如 HttpResponse, WebSocketFrame 等)。
 */
class IResponse {
public:
    virtual ~IResponse() = default;

protected:
    IResponse() = default;
    IResponse(const IResponse&) = delete;
    auto operator=(const IResponse&) -> IResponse& = delete;
    IResponse(IResponse&&) = delete;
    auto operator=(IResponse&&) -> IResponse& = delete;
};

using ResponsePtr = std::unique_ptr<IResponse>;