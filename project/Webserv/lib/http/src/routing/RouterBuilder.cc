// in lib/http/src/routing/RouterBuilder.cc
#include "http/routing/RouterBuilder.hpp"

#include "Status.hpp"
#include "config/Config.hpp"
#include "http/core/Message.hpp"
#include "http/handlers//FunctionHandler.hpp"
#include "http/handlers/RedirectHandler.hpp"
#include "http/handlers/StaticFileHandler.hpp"
#include "http/handlers/UploadHandler.hpp"
#include "http/routing/MethodRouter.hpp"
#include "http/routing/PrefixRouter.hpp"
#include "http/routing/RequestRouter.hpp"
#include "logger.hpp"

#include <algorithm>
#include <magic_enum/magic_enum.hpp>
#include <set>

namespace http {

auto RouterBuilder::build(const ServerConfig& server_config) -> std::unique_ptr<IRequestDispatcher>
{
    // 1. 创建顶级的“管道”路由器
    auto main_router = std::make_unique<RequestRouter>();

    // 2. 对 locations 按路径长度进行降序排序，确保最长前缀优先匹配
    auto locations = server_config.locations;
    std::ranges::sort(locations, [](const auto& a, const auto& b) {
        return a.path.length() > b.path.length();
    });

    for (const auto& loc_conf : locations) {
        // 3. 为每个 location 创建一个专用的处理器
        std::unique_ptr<IRequestHandler> location_handler;

        if (loc_conf.return_directive) {
            // --- Case A: 这是一个重定向 location ---
            const auto& redir = *loc_conf.return_directive;
            location_handler = std::make_unique<RedirectHandler>(redir.code, redir.url);

        } else {
            // --- Case B: 这是一个基于文件系统的 location (常规) ---
            auto method_router = std::make_unique<MethodRouter>();

            // 确定允许的方法 (如果 location 未指定，则继承 server 的默认)
            // (我们先假设默认允许 GET/HEAD/DELETE)
            std::set<http::Method> allowed_methods;
            if (loc_conf.methods) {
                for (const auto& method_str : *loc_conf.methods) {
                    // (这里需要一个从 string 到我们 C++ enum 的转换)
                    // 我们可以用 magic_enum::enum_cast，或者一个简单的 map/if-else
                    if (method_str == "GET")
                        allowed_methods.insert(http::Method::GET);
                    else if (method_str == "POST")
                        allowed_methods.insert(http::Method::POST);
                    else if (method_str == "DELETE")
                        allowed_methods.insert(http::Method::DELETE);
                    else if (method_str == "PUT")
                        allowed_methods.insert(http::Method::PUT);
                    else if (method_str == "HEAD")
                        allowed_methods.insert(http::Method::HEAD);
                }
            } else {
                allowed_methods = { http::Method::GET, http::Method::HEAD };
            }

            // 确定文档根目录 (alias 优先于 root)
            std::filesystem::path document_root = loc_conf.alias.value_or(server_config.root);

            auto static_service = std::make_shared<StaticFileHandler>(
                document_root,
                loc_conf.path);
            auto uploader = std::make_shared<UploadHandler>(document_root); // 上传到 location 的根目录

            // 将 GET/HEAD/DELETE 方法绑定到 static_service 的对应方法
            auto lambda = [static_service](const auto& req) {
                LOG_INFO("🦆New Request : {},{}", magic_enum::enum_name<http::Method>(reinterpret_cast<const Request&>(req).method), reinterpret_cast<const Request&>(req).uri);
                return static_service->handleRequest(req);
            };
            auto lambda2 = [uploader](const auto& req) {
                LOG_INFO("🦆New Request : {},{}", magic_enum::enum_name<http::Method>(reinterpret_cast<const Request&>(req).method), reinterpret_cast<const Request&>(req).uri);
                return uploader->handleRequest(req);
            };

            if (allowed_methods.contains(http::Method::GET)) {
                method_router->addHandler(Method::GET, std::make_unique<ResponseFunctionHandler<decltype(lambda)>>(lambda));
            }
            if (allowed_methods.contains(http::Method::HEAD)) {
                method_router->addHandler(Method::HEAD, std::make_unique<ResponseFunctionHandler<decltype(lambda)>>(lambda));
            }
            if (allowed_methods.contains(http::Method::DELETE)) {
                method_router->addHandler(Method::DELETE, std::make_unique<ResponseFunctionHandler<decltype(lambda)>>(lambda));
            }
            if (allowed_methods.contains(http::Method::POST)) {
                method_router->addHandler(Method::POST, std::make_unique<ResponseFunctionHandler<decltype(lambda2)>>(lambda2));
            }

            location_handler = std::move(method_router);
        }

        // 4. 将配置好的 location 处理器用一个“前缀路由器”包装
        auto prefix_router = std::make_unique<PrefixRouter>();
        prefix_router->addHandler(loc_conf.path, std::move(location_handler));

        // 5. 将这个 location 链插入到主管道
        main_router->use(std::move(prefix_router));
    }

    return main_router;
}

} // namespace http