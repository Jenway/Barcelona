#pragma once

#include "ISinker.hpp"
#include "Status.hpp"
#include <expected>
#include <memory>
#include <string_view>

namespace protocol {

/**
 * @class IHandler
 * @brief 一个纯虚基类，定义了“应用层协议处理器”的策略接口。
 *
 * IHandler 的具体实现（如 HttpHandler）负责解析从客户端收到的字节流，
 * 并生成一个 IResponse 对象作为响应。
 */
class IHandler {
public:
    virtual ~IHandler() = default;

    /**
     * @brief 当 Connection 从网络读到新数据时调用此方法。
     * @param data 新接收到的数据的一个视图。
     */
    virtual void onData(std::string_view data) = 0;
    virtual void onReadEOF() = 0;

    /**
     * @brief 当 socket 变为可写时被调用。
     * @param sinker 一个 I/O 执行器，Handler 可以用它来发送数据。
     * @return 返回写入操作的结果，用于指导 Connection 的状态转换。
     */
    virtual auto onWriteReady(ISinker& sinker)
        -> std::expected<core::WriteResult, std::error_code>
        = 0;

    /**
     * @brief 返回协议处理器当前的状态。
     */
    [[nodiscard]] virtual auto getStatus() const -> core::protocol::Status = 0;

protected:
    IHandler() = default;
    IHandler(const IHandler&) = delete;
    auto operator=(const IHandler&) -> IHandler& = delete;
    IHandler(IHandler&&) = delete;
    auto operator=(IHandler&&) -> IHandler& = delete;
};

// 为方便使用，定义一个标准的智能指针类型别名
using HandlerPtr = std::unique_ptr<IHandler>;

} // namespace protocol
