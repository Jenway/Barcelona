#pragma once
#include "http/interfaces/IRequestHandler.hpp"
#include <functional>

namespace http {

// 一个通用的 IRequestHandler 实现，用于包装任意的 lambda 或函数对象
class FunctionHandler : public IRequestHandler {
public:
    using HandlerFunc = std::function<std::expected<Response, std::error_code>(const Request&)>;

    explicit FunctionHandler(HandlerFunc func)
        : _func(std::move(func))
    {
    }

    auto handleRequest(const Request& req) -> std::expected<Response, std::error_code> override
    {
        // 如果我们包装的函数返回 Response，而接口需要 expected，这里需要适配
        // 但由于我们 StaticFileHandler 等都返回 Response，我们需要一个适配器
        // 为了简单，我们先假设包装的函数都返回 expected
        return _func(req);
    }

private:
    HandlerFunc _func;
};

// 一个更通用的版本，可以适配返回 Response 的函数
template <typename F>
class ResponseFunctionHandler : public IRequestHandler {
public:
    explicit ResponseFunctionHandler(F func)
        : _func(std::move(func))
    {
    }
    auto handleRequest(const Request& req) -> std::expected<Response, std::error_code> override
    {
        return _func(req); // 直接返回 Response，可以隐式转换为 expected<Response, ...>
    }

private:
    F _func;
};

} // namespace http