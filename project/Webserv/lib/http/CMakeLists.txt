# lib/http/CMakeLists.txt

# 1. 声明一个纯接口的聚合库
add_library(http_agg INTERFACE)
add_library(webserv::http ALIAS http_agg)

# 2. 添加对内部分散模块的依赖
target_link_libraries(http_agg INTERFACE
    webserv::http_protocol
    webserv::http_dispatch
    webserv::http_handlers
)

# 3. 递归地添加子模块
add_subdirectory(protocol)
add_subdirectory(routing)
add_subdirectory(handlers)
add_subdirectory(common)
add_subdirectory(utils)
