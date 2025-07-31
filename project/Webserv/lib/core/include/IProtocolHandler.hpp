#pragma once

#include "IResponse.hpp"
#include "Status.hpp"
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

    /**
     * @brief 当 Connection 准备好发送数据时，调用此方法来获取一个待发送的响应。
     * @return 如果一个完整的响应已经准备好，则返回一个包含 IResponse 的智能指针。
     *         如果协议还在等待更多数据或处理中，则返回 nullptr。
     */
    [[nodiscard]] virtual auto produceResponse() -> ResponsePtr = 0;

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
