# WebServer 开发日志

WebServer：一个基于现代 C++ 构建的、事件驱动的 HTTP/1.1 服务器。

## 项目结构

项目采用模块化的目录结构，将不同的功能组件分离到各自的库中，并通过 CMake 进行组织。

- `lib/`: 存放所有核心和辅助功能库。
    - `common/`: 项目级的通用类型定义，如错误码。
    - `logger/`: 高性能日志库。
    - `tcp/`: 底层 TCP 网络组件。
    - `utils/`: 通用的辅助函数。
- `src/`: 主程序入口 `main.cc`。
- `tests/`: 存放所有模块的单元测试。
- `doc/`: 项目文档。

____

## 核心架构

在完成了底层 TCP 组件的封装后，我们开始构建服务器的核心逻辑层。为了实现极致的解耦和可测试性，我们采用**依赖倒置原则**，设计了一套由**纯粹抽象接口**驱动的架构。

在这个模型中，`Connection` 类本身不执行任何 I/O 操作或协议解析，而是扮演一个纯粹的**事件调度员 (Event Dispatcher)** 和**状态机 (State Machine)**。

### 核心抽象

1.  **`IHandler` (`core`)**:
    一个“协议处理器”的**策略接口**。它的具体实现（如未来的 `HttpHandler`）封装了所有的应用层逻辑。它负责解析收到的数据，维护协议状态，并**直接指挥** `ISinker` 来发送响应。

2.  **`ISource` (`io`)**:
    一个“数据源”接口，封装了所有的底层**读 I/O** 逻辑。它的具体实现（如 `TcpSource`）知道如何从一个 `Socket` 中高效地读取数据。

3.  **`ISinker` (`io`)**:
    一个“数据汇”接口，它将自身作为一个**I/O 工具箱**暴露出来。它不关心要发送的数据是什么，只提供一组纯粹的、底层的 I/O 写操作**原语 (Primitives)**，如 `write()` 和 `sendfile()`。

4.  **`Connection` (`core`)**:
    `Connection` 是这个架构的中心。它像一个纯粹的指挥官，协调其他组件工作，但**不参与具体业务**：
    - **持有依赖**: 它拥有一个 `Socket`（资源句柄），以及 `IHandler`、`ISource` 和 `ISinker` 的实例。
    - **调度事件**: 它的核心是 `onReadable()` 和 `onWritable()`。
        - `onReadable()`: 调用 `ISource` 读取数据，然后将数据**委托**给 `IHandler`。
        - `onWritable()`: 将 `ISinker` 这个“I/O工具箱”**递给** `IHandler`，让 `IHandler` 自己决定如何使用它来发送数据。
    - **驱动状态机**: `Connection` 根据 `IHandler` 返回的状态（`WantRead`, `WantWrite` 等）来更新自己的 I/O 状态（`READING`, `WRITING`），并以此决定向上层事件循环注册什么事件。

这种设计实现了完美的关注点分离和单向依赖流 (`http` -> `core` -> `io` <- `tcp`)，使得每个组件都可以被独立地替换和测试。


### TCP 网络层 (`lib/tcp`)

TCP 模块提供了一套面向对象的、基于 RAII 的底层网络操作封装。采用 `std::expected` 进行错误传递，为上层事件循环提供健壮、清晰的接口。

- **`FileDescriptor`**
    - 一个纯粹的 RAII 包装器，唯一职责是管理文件描述符的生命周期，确保在对象析构时自动调用 `close()`。
- **`Socket`**
    - 基于 `FileDescriptor`，封装了核心的套接字操作。
    - **无异常设计**: 所有可能失败的操作 (如 `create`, `bind`, `accept`) 均返回 `std::expected<T, std::error_code>`，强制调用者处理错误路径。
    - **现代 API 优先**: 在支持的平台（Linux）上，通过 `socket()` 和 `accept4()` 的标志位，以**原子操作**的方式设置 `O_NONBLOCK` 和 `O_CLOEXEC`，也同时提供了基于 `fcntl` 的可移植回退方案。
    - **默认非阻塞**: 所有创建的套接字默认为非阻塞模式。
- **`Acceptor`**
    - 一个高级组件，它将服务器监听的固定流程（`create` -> `bind` -> `listen`）封装在一个工厂函数 `create()` 中，向上层提供了一个简洁的 `accept()` 接口。
- **`TcpSource` / `TcpSinker`**: `ISource` 和 `ISinker` 接口的具体 TCP 实现，被放在 `tcp` 模块中，体现了“接口定义”与“具体实现”的分离。

____

## 辅助模块

### 统一错误处理 (`lib/common`)

为了实现全项目统一的、类型安全的错误处理，我们设计了一套基于 `std::error_category` 的系统。

- **`ErrorCode` 枚举**: 定义在 `ErrorCode.hpp` 中，是整个项目的“错误字典”，包含了所有非系统级的、特定于我们应用（如网络、配置、HTTP）的错误码。
- **自定义 `error_category`**: 我们实现了自己的 `WebServerCategory`，让 `ErrorCode` 枚举无缝融入 C++ 的 `<system_error>` 框架。这使得我们可以直接将自定义的错误码与 `std::error_code` 对象进行比较，如 `if (ec == ErrorCode::Net_InvalidAddress)`。
- **与 `magic_enum` 集成**: 借助 `magic_enum` 库，我们实现了 `enum` 到字符串的零样板代码、编译期转换，极大地简化了日志记录和调试。

### 日志库 (`lib/logger`)

日志库的核心功能完全基于 **`{fmt}`** 库。

> `{fmt}` 提供了编译期类型安全检查和很好的运行时性能。

每一条日志都自动包含了丰富的上下文信息，便于快速定位问题：

- **本地时间戳** (精确到毫秒)。
- **日志级别** (TRACE, DEBUG, INFO, WARN, ERROR)，并带有终端颜色高亮。
- **源码位置**：通过 `std::source_location` (C++20) 获取文件名、行号和函数名。
- **相对路径**：自动将源码的绝对路径转换为相对于项目根目录的路径，使日志更整洁。

为了方便调试和日志归档，程序在启动时会通过 POSIX 函数 `isatty()` 自动检测其标准输出是否连接到交互式终端。

- 如果是终端，则**启用**颜色高亮。
- 如果输出被重定向到文件或管道（例如 `| tee log.txt`），则**自动禁用**颜色，以保证日志文件的纯净。

___

## 测试模块 (`tests/`)

我们使用 **GoogleTest** 作为测试框架，并为每个模块编写了独立的单元测试。

### 测试 `Logger` 模块

- **冒烟测试**: 验证所有日志宏在调用时都能正常编译且不引发崩溃。
- **输出验证测试**: 通过 GTest 内置的工具捕获 `stdout` 和 `stderr`，并使用 GTest Matchers 精确断言日志的输出内容、格式和级别过滤行为的正确性。测试期间会自动禁用颜色，确保测试的稳定性和可靠性。

### 测试 `TCP` 模块

- **正常流程测试**: 验证 `Acceptor` 能成功创建并接受一个（在另一线程中模拟的）客户端连接。
- **应用错误测试**: 验证当提供无效输入（如非法 IP 地址）时，函数能返回我们自定义的、正确的 `ErrorCode`。
- **系统状态测试**: 验证在非阻塞模式下，当没有连接到来时，`accept()` 能立即返回并附带一个可被识别为 `EAGAIN` 的 `std::error_code`。

### 测试 `core/Connection` 模块

我们使用 **GoogleMock** 为 `IHandler`, `ISource`, `ISinker` 创建了**模拟对象 (Mock Objects)**。这使得我们可以**在完全不涉及真实网络 I/O 的情况下**，对 `Connection` 的状态机逻辑进行全面、独立的单元测试，覆盖了从正常读写循环到各种错误路径的所有关键场景。

___

## 开发工作流

项目采用基于 CMake 和 vcpkg 的现代化、模块化开发工作流。

- **构建系统**: 使用现代 CMake，通过 `target_link_libraries` 的 `PUBLIC` 和 `INTERFACE` 属性实现了依赖的自动传递。
- **依赖管理**: 使用 `vcpkg` 管理所有第三方库 (`fmt`, `gtest`, `magic_enum`)，确保了开发环境的一致性和可复现性。
- **编译优化**: 通过 `target_precompile_headers` 将 `logger.hpp` 设置为预编译头，以加速整体编译过程。

项目配备了三个便捷的 shell 脚本：

- `build.sh`: 重新生成构建系统并编译。
- `test.sh`: 一键编译并运行所有单元测试。
- `debug.sh`: 编译并运行主程序，同时将日志输出到屏幕和 `log.txt` 文件。