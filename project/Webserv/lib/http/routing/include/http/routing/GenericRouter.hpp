// in lib/http/dispatch/include/http/routing/GenericRouter.hpp
#pragma once

#include "http/common/HttpStatus.hpp"
#include "http/interfaces/IRequestHandler.hpp"
#include "http/utils/ResponseFactory.hpp"
#include <map>
#include <memory>
#include <string>

namespace http::routing {

// --- 策略定义 ---

// 策略 1: 最长前缀匹配
struct LongestPrefixMatch {
    // 函数对象，接收 handler 容器和请求 URI
    IRequestHandler* operator()(
        const std::map<std::string, std::unique_ptr<IRequestHandler>>& handlers,
        const std::string& uri) const
    {
        std::string best_match_prefix;
        IRequestHandler* best_handler = nullptr;
        for (const auto& [prefix, handler] : handlers) {
            if (uri.starts_with(prefix) && prefix.length() > best_match_prefix.length()) {
                best_match_prefix = prefix;
                best_handler = handler.get();
            }
        }
        return best_handler;
    }
};

// 策略 2: 精确 Key 匹配
template <typename Key>
struct ExactKeyMatch {
    // 函数对象，接收 handler 容器和请求 Key
    IRequestHandler* operator()(
        const std::map<Key, std::unique_ptr<IRequestHandler>>& handlers,
        const Key& request_key) const
    {
        if (auto it = handlers.find(request_key); it != handlers.end()) {
            return it->second.get();
        }
        return nullptr;
    }
};

// --- 通用路由器模板 ---

template <
    typename Key, // 用于匹配的键 (e.g., std::string)
    typename KeyFromRequest, // 如何从 Request 中提取 Key
    typename Matcher // 使用哪种匹配策略
    >
class GenericRouter : public IRequestHandler {
public:
    void addHandler(Key key, std::unique_ptr<IRequestHandler> handler)
    {
        _handlers[key] = std::move(handler);
    }

    auto handleRequest(const Request& request) -> std::expected<Response, std::error_code> override
    {
        // 1. 从请求中提取用于匹配的 Key
        const Key request_key = KeyFromRequest {}(request);

        // 2. 使用匹配策略，在容器中查找最佳的 handler
        IRequestHandler* best_handler = Matcher {}(_handlers, request_key);

        if (best_handler != nullptr) {
            return best_handler->handleRequest(request);
        }

        // 3. 根据 Key 的类型，返回不同的 fallback 响应
        if constexpr (std::is_same_v<Key, Method>) {
            return responses::createStockResponse<StatusCode::MethodNotAllowed>();
        } else {
            return responses::createStockResponse<StatusCode::NotFound>();
        }
    }

private:
    std::map<Key, std::unique_ptr<IRequestHandler>> _handlers;
};

// --- 从 Request 中提取 Key 的工具 ---
struct GetUri {
    std::string operator()(const Request& req) const { return req.uri; }
};
struct GetMethod {
    Method operator()(const Request& req) const { return req.method; }
};

// --- 最终的类型别名 ---
// 现在，PrefixRouter 和 MethodRouter 都只是这个通用模板的别名！
using PrefixRouter = GenericRouter<std::string, GetUri, LongestPrefixMatch>;
using MethodRouter = GenericRouter<Method, GetMethod, ExactKeyMatch<Method>>;

} // namespace http::routing