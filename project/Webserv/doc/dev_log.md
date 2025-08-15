# WebServer 开发日志

WebServer：一个基于现代 C++ 构建的、事件驱动的 HTTP/1.1 服务器。

## 项目结构

项目采用模块化的目录结构，将不同的功能组件分离到各自的库中，并通过 CMake 进行组织。

- `lib/`: 存放所有核心和辅助功能库。
    - `common/`: 项目级的通用类型和错误处理框架。
    - `config/`: 负责解析和验证 `config.json` 的配置库。
    - `core/`: 服务器核心 Reactor 逻辑，如 Connection 、Channel 和 Poller。
    - `http/`: HTTP 应用层库
    - `logger/`: 日志库。
    - `tcp/`: 底层 TCP 网络组件。
    - `utils/`: 通用的辅助函数。
- `src/`: 主程序 `Server` 类和 `main` 函数等。
- `tests/`: 存放所有模块的单元测试。
- `doc/`: 项目文档。

---

## 核心架构

### 并发模型：主-从 (Main-Worker) 多线程 Reactor

#### 1. 主线程 (Master): `Server` 类

`Server` 是一个纯粹的**监听器 (Listener)** 和**分发器 (Dispatcher)**。

-   **职责**:
    -   **初始化**: `main` 函数通过静态工厂 `Server::create` 创建唯一的 `Server` 实例。`Server::setup` 方法负责解析配置、创建 `Reactor` 工作线程池，并设置信号处理。
    -   **监听**: `Server` 拥有**唯一**的 `Acceptor` 实例，并在**自己专属的 `Poller`** (事件循环) 中，只监听新连接的到来和终止信号。
    -   **分发**: 当 `Acceptor` 接收到一个新的 `Socket` 时，`Server::onNewConnection` 方法被触发。它**不会**自己处理这个连接，而是通过一个**轮询 (Round-robin)** 算法，选择一个工作线程，并通过该线程的**线程安全队列**，将 `Socket` 的所有权**异步地**分发出去。

主线程的事件循环 (`Server::run`) 非常轻量，它只做两件事：等待新连接，以及等待终止信号。

#### 2. 工作线程 (Workers): `Reactor` 类

每个`Reactor`实例都代表一个**独立的工作单元**，运行在自己的 `std::jthread` 中。

-   **职责**:
    -   **事件循环**: 每个 `Reactor` 拥有**自己独立的 `Poller` (`epoll` 实例)**。这避免了多线程操作同一个 `epoll` 实例时可能产生的锁竞争，是实现高性能的关键。
    -   **任务处理**: `Reactor::run` 的事件循环，除了调用 `poller_.pollOnce()` 等待 I/O 事件，还会检查自己的**任务队列 (`ThreadSafeQueue<Socket>`)**。
    -   **连接管理**: 当从队列中获取一个新的 `Socket` 后，`Reactor` 内部的 `onNewConnection` 方法被调用。它负责创建 `Connection` 对象、`Channel`，并将它们注册到**自己**的 `Poller` 上。
    -   **I/O 处理**: `Reactor` 全权负责处理它所持有的所有连接上的所有 I/O 事件（读、写、关闭）。

#### 数据流：一次请求的完整旅程

1.  客户端发起 TCP 连接。
2.  `Server` (主线程) 的 `Poller` 唤醒，`Acceptor` 接受连接，得到一个 `Socket`。
3.  `Server::onNewConnection` 通过轮询，将 `Socket` `push` 进 `Reactor #N` 的任务队列。
4.  `Reactor #N` (工作线程) 在自己的循环中，从队列 `pop` 出这个 `Socket`。
5.  `Reactor #N` 为该 `Socket` 创建一个 `Connection`，并将其纳入自己的 `Poller` 开始管理。
6.  后续的所有 HTTP 请求-响应，都完全在这个 `Reactor #N` 线程内部完成，与主线程和其他 `Reactor` 线程**完全隔离**。

### Reactor 核心架构 (`lib/core`)

本模块是 `WebServer` 的**心脏和骨架**，它定义了整个服务器的**事件驱动模型**和**连接生命周期管理**。其设计严格遵循**依赖倒置**和**单一职责**原则，实现了极致的解耦和可测试性。

#### 1. Reactor 事件循环

服务器的核心是一个基于 **Reactor 设计模式**的事件循环。

-   **`Poller` (事件分发器)**: 作为 Reactor 的核心，它是对 Linux 高性能 **`epoll`** 的一个现代 C++ 封装。它负责监控所有文件描述符的 I/O 事件，并在事件发生时，**高效地调用**已注册的回调函数。`Poller` 的接口经过抽象，使其上层逻辑与具体的 `epoll` 实现解耦。

-   **`Channel` & `bind_to` (声明式事件绑定)**: 为了将**“什么应该发生”**（业务回调）与**“如何让它发生”**（`epoll` 注册）分离开来，我们构建了一套更高阶的声明式抽象。
    -   **`Channel`** 是一个纯粹的**“事件蓝图”**，它只负责描述一个 `fd` 与其 `onReadable`/`onWritable` 回调之间的关系。
    -   **`bind_to`** 是一个基于 `tag_invoke` 的**自定义点对象 (CPO)**，它提供了一个统一的动词，负责将 `Channel` 的“蓝图”配置应用到 `Poller` 这个“引擎”上。

#### 2. `Connection`: 连接状态机与协调者

`Connection` 类是整个架构的中心枢纽。它本身不执行任何具体的 I/O 操作或协议解析，而是扮演一个纯粹的**事件调度员**和**状态机**。

-   **职责**:
    -   **持有依赖**: 它拥有一个 `Socket`（资源句柄），以及三个核心的抽象接口实例：`IHandler`（协议处理器）、`ISource`（数据源）和 `ISinker`（数据汇）。
    -   **调度事件**: 它的 `onReadable()` 和 `onWritable()` 方法，在被 `Poller` 唤醒后，负责分别调用 `ISource` 来读取数据和调用 `IHandler` 来写入数据。
    -   **驱动状态机**: `Connection` 根据 `IHandler` 返回的 `protocol::Status` 指令（`WantRead`, `WantWrite`, `Finished`），来更新自己的内部状态（`READING`, `WRITING`, `CLOSING`），并以此决定下一步应该向 `Poller` 注册什么 I/O 事件。

### 3. 健壮的连接生命周期：优雅关闭

为了确保在各种边界情况下的稳定性和数据完整性，`Connection` 的状态机实现了一套健壮的**“优雅关闭”**流程。

-   当协议处理器指示连接应关闭时 (`Finished` 状态)，`Connection` 不会立即 `close()`，而是先调用 `socket.shutdownWrite()` **半关闭**连接，向客户端发送 `FIN`。
-   随后，`Connection` 进入一个新的 **`CLOSING`** 状态，在此状态下它会继续监听并“排干”客户端可能仍在发送的数据，直到接收到客户端的 `FIN` 报文（表现为 `read()` 返回 `EOF`）。
-   只有在确认 TCP 四次挥手基本完成后，`Connection` 才会最终进入 `CLOSED` 状态，并通过 RAII 机制安全地回收文件描述符。

这个机制完美地解决了在错误场景（如 `413 Payload Too Large`）下，客户端因连接过早断开而无法收到响应的问题。

> **[阅读 `core` 模块的详细设计...](./reactor-core.md)**

### HTTP 模块 (`lib/http`)

为了实现一个功能完备且高度可扩展的 HTTP/1.1 服务，我们构建了一个**模块化的微框架体系**。该体系通过一个顶层的**聚合模块 (`lib/http`)**，将多个职责单一的子模块组合起来，对外提供统一的 `webserv::http` 目标。

#### 1. `lib/http/protocol`: 协议编解码引擎

本模块是服务器的**底层协议引擎**，负责 **字节流 ↔ 消息对象** 的双向转换，其设计思想类似于 Boost.Beast。

-   **`HttpProtocolHandler`**: 作为实现了 `protocol::IHandler` 接口的核心协调者，它驱动着整个协议状态的流转，并负责最终的 **Keep-Alive** 决策。
-   **`RequestParser`**: 一个健壮的、基于 FSM 的**验证型流式解析器**。它能够在解析阶段就执行 **URI 规范化**、**Body 大小限制**等安全检查，并完整支持 **`Transfer-Encoding: chunked`** 的“解分块”逻辑。
-   **`ResponseWriter`**: 一个高效的响应序列化器，能智能地根据 `Response.body` 的类型（内存或文件）选择 `write()` 或 **`sendfile()`** (零拷贝) 进行发送。

#### 2. `lib/http/routing`: 路由与分发框架

本模块是一个**纯头文件 (Header-Only)** 的、可组合的路由工具集，负责将请求精确分发给正确的业务处理器。
-   **核心**:
    -   基于**策略模式**的**`GenericRouter` 模板**，将匹配逻辑抽象为可插拔的策略，以**零重复代码**的方式生成了 `PrefixRouter` (最长前缀匹配) 和 `MethodRouter` (按方法精确匹配)。
    -   基于**责任链模式**的**`RequestRouter`**，它扮演着“管道工”的角色，能够按顺序串联多个处理器，直到请求被成功处理。

#### 3. `lib/http/handlers`: 具体业务处理器

本模块提供了一系列**具体的、可插拔的业务逻辑实现**，是我们服务器功能的“血肉”。

-   **`StaticFileHandler`**: 提供了完整的静态资源服务 (`GET`/`HEAD`/`DELETE`)。
-   **`UploadHandler`**: 实现了文件上传 (`POST`/`PUT`)。
-   **`CgiHandler`**: 实现了 CGI/1.1 规范，作为与外部动态脚本交互的网关。
-   **`RedirectHandler`**: 实现了 `3xx` HTTP 重定向。

#### 4. `src/RouterBuilder`: 应用组装层

为了将**声明式的配置文件 (`config.json`)** 与**程序化的对象图 (我们的处理器)** 连接起来，我们将构建逻辑提升到了最终的**应用程序层** (`src/`)。`RouterBuilder` 负责读取解析后的 `Config` 结构，并调用 `routing` 和 `handlers` 模块提供的组件，**自动地**构建出服务器所需的、完整的顶层请求分发器。

> **[阅读 `http` 模块的详细设计...](./protocol-http.md)**

## IO 封装与 TCP 网络层

### 通用工具（`lib/utils`）

`utils` 提供全项目可用的基础工具，与业务逻辑无关。

* **`FileDescriptor`**：RAII 封装文件描述符

  * 自动在析构时 `close()`，防止泄漏
  * 仅移动类型，确保唯一所有权
  * 提供 `get()`, `release()`, `isValid()` 等方法

* **`FileUtils`**：安全文件系统操作

  * `resolveSafePath`：安全解析 URI，防止路径遍历
  * `getFileInfo`：获取文件元数据
  * `openFileForReading`：返回由 `FileDescriptor` 管理的文件句柄
  * `getMimeType`：根据扩展名获取 MIME 类型

---

### TCP 网络层（`lib/tcp`）

基于 `utils` 封装现代化 TCP 网络操作。

* **`Endpoint`**：类型安全的网络地址

  * 封装 IP 字符串和端口
  * 可转换为底层 `sockaddr_in`

* **`Socket`**：通信端点

  * RAII 管理套接字生命周期
  * API 纯粹：移除 `bind()`/`listen()` 等仅监听相关方法
  * 提供连接管理 (`shutdownWrite`, `close`) 与 TCP 配置 (`setTcpNoDelay`, `setLinger`, `setKeepAlive`)
  * `create()` 工厂方法，优先使用 `accept4()` + `SOCK_CLOEXEC | SOCK_NONBLOCK` 创建非阻塞套接字

* **`Acceptor`**：专职监听器

  * 封装 `socket → bind → listen` 流程
  * 持有实际监听句柄，不伪装成 `Socket`
  * `onAccept()` 循环 `accept()` 所有等待连接，适配边缘触发 (ET) I/O
  * 解耦事件循环，通过 `setAcceptHandler` 将新 `Socket` 传递给上层

* **`TcpSource` / `TcpSinker`**：TCP I/O 接口实现

  * 封装 `::read()`, `::write()`, `::sendfile()`
  * 支持非阻塞操作，结合 `std::expected` 统一错误处理


## 配置、错误处理与日志

### 配置文件系统

为了使服务器行为高度可定制，我们引入了一个基于 **JSON** 的、严格类型化的配置系统。

-   **解析引擎**: **`nlohmann/json`** 库作为核心解析器，提供强大且安全的 JSON 操作能力。所有配置都从一个指定的配置文件（默认为 `config.json`）加载。
-   **强类型配置结构**:
    -   在 `lib/config/include/config/Config.hpp` 中，我们定义了一系列与 `config.json` 层级结构严格对应的 C++ `struct`（如 `Config`, `ServerConfig`, `LocationConfig`）。
    -   这种设计利用 C++ 的类型系统，在**编译期**就能对配置结构进行约束，并为开发提供了清晰的自动补全和静态分析支持，极大地减少了运行时因配置错误导致的问题。
-   **健壮的解析与验证**:
    -   `ConfigLoader.cc` 中实现的 `parse_config` 系列函数，负责将 `nlohmann::json` 对象安全地转换为我们强类型的 C++ `struct`。
    -   解析器会严格验证所有**必需字段**的存在性和类型（如 `listen`, `root`），并能优雅地处理**可选配置项**（如 `alias`, `index`）。
    -   包含一个专用的 `parse_body_size` 工具函数，可以将人类可读的字符串（如 `"10MB"`, `"2GB"`）安全地转换为 `size_t` 字节数，提升了配置文件的易用性。
    -   任何解析或验证失败，都会通过项目的**分层错误处理框架**返回一个附带详细上下文信息的 `std::system_error`，确保了启动失败时能够快速定位问题。

---

### 错误处理框架

项目采用了一套分层的、以**可诊断性**为核心的错误处理策略，以在**性能**和**信息完整性**之间取得最佳平衡。

### 统一错误码

-   **`ErrorCode` 枚举**: 定义在 `ErrorCode.hpp` 中，是整个项目的“错误字典”，包含了所有非系统级的、**通用**的应用错误码（如网络、配置）。
-   **自定义 `error_category`**: 我们实现了自己的 `WebServerCategory`，让 `ErrorCode` 枚举无缝融入 C++ 的 `<system_error>` 框架。这使得我们可以直接、类型安全地比较 `std::error_code` 对象（如 `if (ec == ErrorCode::Net_InvalidAddress)`），并能通过 `magic_enum` 自动获取其字符串表示。

### 分层错误返回

-   **API 边界 / 冷路径 -> `std::system_error`**:
    -   对于服务器启动、资源创建、配置加载等一次性操作，函数返回 `std::expected<T, std::system_error>`。`system_error` 对象打包了 `error_code` 和详细的、包含源码位置的**上下文消息**，确保了在关键路径失败时能提供最丰富的诊断信息。
-   **内部循环 / 热路径 -> `std::error_code`**:
    -   对于高频的 I/O 操作（如 `Socket::accept`），函数返回轻量级的 `std::expected<T, std::error_code>`，避免了在性能热点上因字符串操作和内存分配带来的开销。

### 自动化错误聚合 (`error::to_unexpected` CPO)

为了将分层策略优雅地固化为代码实践，我们实现了一个 header-only 的 CPO (Customization Point Object) 框架 (`Error.hpp`)。

-   `error::to_unexpected`: 一个统一的、可扩展的错误返回接口。它通过一系列重载，可以极其方便地将 `errno`、`std::errc` 或自定义 `ErrorCode` 与上下文消息自动打包成一个 `std::unexpected<std::system_error>`。
-   `error::to_unexpected_code`: 一个用于热路径的轻量级版本，只返回封装了 `errno` 或 `std::errc` 的 `std::unexpected<std::error_code>`。
这套框架将“手动聚合错误信息”的样板代码完全自动化，极大地提升了代码的简洁性和健壮性。

---

## `lib/logger`: 现代化日志库

日志库是整个项目可观测性 (Observability) 的基石，它基于 **`{fmt}`** 库和 C++20 的新特性构建。

-   **核心特性**:
    -   **高性能格式化**: 完全基于 `{fmt}` 库，提供编译期格式化字符串检查和极高的运行时性能。
    -   **丰富的上下文**: 每一条日志都**自动**包含了丰富的元数据，便于快速定位问题：
        -   **高精度时间戳** (`HH:MM:SS.ss`)。
        -   **日志级别** : 提供运行时或者编译时等级过滤 (TRACE, DEBUG, INFO, WARN, ERROR)，并带有**终端颜色高亮**。
        -   **源码位置**: 通过 **`std::source_location` (C++20)** 自动获取文件名、行号和函数名。
        -   **相对路径**: 自动将源码的绝对路径转换为相对于项目根目录的路径，使日志更整洁。
    -   **`consteval` & `constexpr`**: 大量使用了编译期计算来处理日志级别、颜色、路径等，将运行时开销降至最低。
-   **智能输出**:
    -   程序在启动时会通过 `isatty()` 自动检测其输出是否连接到交互式终端，并据此**自动启用或禁用**颜色高亮，确保了日志文件或管道输出的纯净性。
-   **深度集成**:
    -   通过为 `std::error_code`, `std::system_error` 以及我们项目中所有重要的 `enum` 类型（如 `ConnectionState`, `http::Method`）提供 `fmt::formatter` 特化，我们可以直接将这些复杂对象传递给日志宏，自动打印出结构化的、信息丰富的日志，极大地提升了调试效率。


## 开发工作流与测试策略

为了保证开发效率和代码质量，`WebServer` 项目采用了一套现代化的、高度自动化的开发工作流和分层测试策略。

---
### I. 构建与依赖管理

项目完全基于 **CMake** 和 **vcpkg** 构建，确保了开发环境的一致性和可复现性。

-   **构建系统 (`CMake`)**:
    -   **现代化实践**: 我们全面采用现代 CMake 的特性。通过定义**别名目标 (Alias Targets)**（如 `webserv::core`, `webserv::http`），模块间的依赖关系在 `CMakeLists.txt` 中变得极其清晰和规范。
    -   **模块化**: 项目被划分为多个独立的子模块，每个模块都有自己的 `CMakeLists.txt`。通过 `target_link_libraries` 的 `PUBLIC`, `INTERFACE`, `PRIVATE` 属性，实现了依赖项和头文件路径的**自动传递**，避免了手动管理 include 路径的繁琐。
    -   **编译优化**: 通过 `target_precompile_headers` 将 `logger.hpp` 等常用头文件设置为**预编译头 (PCH)**，显著加速了增量编译过程。

-   **依赖管理 (`vcpkg`)**:
    -   所有第三方库（`{fmt}`, `gtest`, `magic_enum`, `nlohmann-json`）都通过 `vcpkg` 进行管理。
    -   这确保了所有开发者都能使用版本一致的、经过预编译的依赖库，极大地简化了项目的初始搭建流程。

-   **开发环境与预设 (`CMakePresets.json`)**:
    -   我们引入了 `CMakePresets.json` (v3) 文件，为各种 IDE (如 CLion, VS Code) 和命令行提供了**标准的、一键式的**配置、构建和测试预设。
    -   开发者无需手动配置 CMake，只需选择一个预设（如 `debug`），即可获得一个包含所有正确工具链设置和环境变量的、可立即工作的开发环境。

#### 便捷脚本

为了进一步简化日常开发，项目配备了三个便捷的 Shell 脚本：
-   `build.sh`: 重新生成构建系统并编译。它会自动检测并优先使用 `ccache` 和 `mold` 链接器（如果可用），以大幅提升重复编译和链接的速度，并默认启用并行编译。
-   `test.sh`: 一键编译并运行所有单元测试和集成测试。
-   `debug.sh`: 编译并运行主程序，同时将日志输出到屏幕和 `log.txt` 文件。

---

### II. 分层测试策略

项目采用了**分层测试**的策略，确保了从最小的代码单元到最终的用户行为，都得到了充分的质量保障。

我们的测试体系主要分为以下三个层次：

#### 1. 单元测试 (Unit Tests)

-   **目标**: 验证**单个类或函数**在**隔离环境**中的行为是否正确。
-   **框架**: **GoogleTest (`GTest`)** 和 **GoogleMock (`GMock`)**。
-   **实例**:
    -   `parser_test.cc`: 独立测试 `RequestParser` 的状态机，向其输入各种字符串，验证其解析结果和返回的错误码是否符合预期。
    -   `router_test.cc`: 通过**模拟 (`Mock`)** `IRequestHandler`，我们只测试 `RequestRouter` 的“责任链”逻辑，而完全不关心它所包含的子处理器的具体行为。
    -   `cgi_handler_test.cc`: 通过模拟 `ICgiRunner`，我们在**不创建真实进程**的情况下，对 `CgiHandler` 解析 CGI 输出的复杂逻辑进行了全面的单元测试。
-   **特点**: 运行速度极快，定位问题精准，是保证代码内在质量的基石。

#### 2. 集成测试 (Integration Tests)

-   **目标**: 验证**多个相互协作的组件**在一起工作时是否正确。
-   **框架**: 同样使用 **GoogleTest**，但减少了 Mock 的使用。
-   **实例**:
    -   **`tcp_io_test.cc`**: 这是我们最核心的集成测试。它在一个独立的线程中启动一个 `Acceptor`，在主线程中创建一个客户端 `Socket` 并与之连接，然后通过这个真实的 TCP 连接发送和接收数据，从而端到端地验证了 `Acceptor`, `Socket`, `Connection`, `TcpSource`, `TcpSinker` 和 `IHandler` (使用一个简单的 `EchoHandler`) 这一整条**核心 I/O 链路**的正确性。
    -   **`static_file_handler_test.cc`**: 这个测试在**真实的临时文件系统**上创建文件和目录，然后调用 `StaticFileHandler`，验证它与 `FileUtils` 模块的协同工作是否正确。

-   **特点**: 比单元测试更接近真实场景，能够发现组件间接口不匹配或交互逻辑错误等问题。

> 在测试过程中，我们解决了一个由 GTest Mock 和链接器行为导致的**底层内存崩溃**问题。通过将测试内部的 Mock 类放入**匿名命名空间**，我们强制了内部链接，从而避免了符号冲突。

#### 3. 端到端测试 (End-to-End, E2E Tests)

-   **目标**: 从**最终用户**的视角，验证整个**已部署的、正在运行的**应用程序的行为是否符合需求。
-   **框架**: **Python (`pytest`, `requests`)**。
-   **实例 (`e2e_tests/test_server.py`)**:
    -   首先在后台启动**完整**的 `WebServer` 可执行程序。
    -   然后，测试脚本像一个**真实的 Web 客户端**（如浏览器），通过网络向 `http://127.0.0.1:8080` 发起真实的 HTTP 请求。
    -   测试用例使用动态的 **`fixture`** 来创建和清理测试文件，消除了对硬编码内容的依赖，使得测试极其健壮和可维护。
    -   它会验证服务器返回的**状态码、头部、内容**是否完全符合预期。
-   **特点**: 这是**最高级别的测试**。它不关心服务器内部的类是如何实现的，只关心服务器作为一个“黑盒”对外提供的服务是否正确。`E2E` 测试的通过，是我们对项目功能**最终的信心来源**，也是我们开发新功能（如 CGI, Keep-Alive）的**“领航员”**。
