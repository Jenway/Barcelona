<!-- doc/protocol-http.md -->
# HTTP 应用层模块 (lib/http/)

本模块实现了 **HTTP/1.1 协议** 的解析、状态管理和响应生成，其最顶层的实现 (`HttpProtocolHandler`) 对外提供 `protocol::IHandler` 接口。

整个 HTTP 模块被设计为**纯粹的应用层逻辑**，它不直接进行任何 I/O 操作，也不关心事件循环、`epoll`、`socket` 等底层细节。它专注于协议的**解析 (Parsing)**、**状态维护 (State Machine)** 和**响应构建 (Response Building)**，并通过 `IHandler` 接口与 `core::Connection` 进行对话。


## lib/http/protocol (协议编解码层)

这是 HTTP 协议的底层引擎，负责 **字节流 ↔ 消息对象** 的双向转换，相当于一个轻量版的 Boost.Beast。它只处理协议语法，不涉及业务逻辑。

### `HttpProtocolHandler`：协议状态协调器

实现了 `protocol::IHandler`，既是协议状态机，也是业务逻辑与底层 I/O 的中介。

* **职责**：

  * 接收 `Connection` 的字节流，驱动 `RequestParser` 解析请求。
  * 调用 `IRequestDispatcher` 传递 `Request` 并获取 `Response`。
  * 根据 Keep-Alive 策略设置连接头部，并将响应绑定到 `ResponseWriter`。
  * 在一次请求-响应结束后，通过 `protocol::Status` 指令控制连接复用或关闭。

### Request Parser：解析字节流为 Request

`http::RequestParser` 是一个 **基于有限状态机（FSM）** 的流式 HTTP 请求解析器，实现了 `IRequestParser` 接口。

它的职责是从客户端字节流中增量解析出完整的 `HttpRequest` 对象，并在解析过程中进行协议一致性和安全性校验。

#### 核心特性

* **流式增量解析**：支持不完整数据输入，多次调用 `parse()` 逐步推进状态。
* **状态机驱动**：解析流程由 `Step` 枚举控制，包括：

  ```
  RequestLine → Headers → Body / ChunkedBody → Completed
  ```
* **健壮性**：对请求行、头部和体的长度、格式、合法性进行严格验证，防止协议攻击和资源滥用。
* **Keep-Alive 检测**：在解析头部阶段判断客户端是否希望保持连接，结果通过 `clientWantsKeepAlive()` 暴露给上层。

#### 解析流程

-   **解析流程**: 它的内部状态机严格按照 HTTP 报文的结构进行迁移：
    1.  **`RequestLine`**: 解析 `METHOD URI VERSION`。
    2.  **`Headers`**: 逐行解析所有请求头，直到遇到空行 (`CRLF`)。
    3.  **`Body`**: 根据 `Content-Length` 或 `Transfer-Encoding` 头部，进入不同的 Body 解析模式。
    4.  **`Completed`**: 当整个请求被成功解析后，进入完成状态。

-   **核心特性与健壮性设计**:
    -   **验证型解析**: 它能在解析的每个阶段，验证输入是否符合 RFC 规范，并在遇到格式错误、非法方法或版本时，提前失败并返回一个精确的 `http::StatusCode` (如 `400 Bad Request`, `501 Not Implemented`)。
    -   **安全检查**:
        *   **Body 大小限制**: 在解析头部时，它会立即检查 `Content-Length` 是否超过了预设的 `client_max_body_size`，如果超过则立刻返回 `413 Payload Too Large`，有效防止了大型 payload 攻击。
        *   **URI 规范化**: 它调用 `utils::normalizeUriPath`，在解析阶段就对 URI 进行清理和安全检查，阻止了路径遍历等攻击。
    -   **Keep-Alive 决策**: 在 `Headers` 解析完毕后，它会根据 HTTP 版本和 `Connection` 头部，预先计算出客户端的 Keep-Alive 意图 (`clientWantsKeepAlive()`)，为后续的连接管理提供决策依据。
    -   **Chunked 支持**: 它完整地实现了对 `Transfer-Encoding: chunked` 的“解分块”逻辑，能够将分块的数据流重新组合成一个连续的请求体。（*具体实现细节将在“杂项”章节中详述*）。

### `ResponseWriter`：响应序列化器

实现 `IResponseWriter`，将 `http::Response` 序列化并写入 `ISinker`（TCP 输出缓冲）。

#### 核心特性

* **两阶段发送**：内部状态机 `SendingHeaders → SendingBody → Finished` 支持异步分阶段发送。
* **流式输出**：非阻塞 I/O 下可多次调用 `writeTo()`，发送未完成部分。
* **智能 Body 处理**：

  * `std::vector<char>`：通过 `ISinker::write()` 发送。
  * `http::FileBody`：通过 `ISinker::sendfile()` 实现零拷贝，提高大文件传输性能。
* **Keep-Alive 决策**：

  * `setKeepAlive(bool)` 接收最终策略。
  * `bind_to()` 在序列化前设置 `Connection` 响应头。
  * `isKeepAlive()` 报告最终连接管理策略。

#### 工作流程

1. **绑定响应**：`bind_to(Response)` 序列化响应行和头部。
2. **写入循环**：`writeTo(ISinker&)` 先发送头部，再发送 Body，完成后进入 `Finished` 状态。
3. **错误与阻塞处理**：所有写操作返回 `std::expected<core::WriteResult, std::error_code>`，未写入数据不会丢失。`Finished` 状态再次调用返回空结果。


## lib/http/handlers（请求处理器）

本模块实现了 `IRequestHandler` 接口，是 Web 服务器功能的**最终执行单元**。每个处理器负责一种具体业务逻辑，通过上层路由 (`routing`) 模块组合起来，构成服务器完整行为。

### `StaticFileHandler`：静态文件服务

处理文件系统上的静态资源请求，支持 `GET`, `HEAD`, `DELETE` 等方法。

* **安全**：通过 `utils::resolveSafePath` 防止路径遍历。
* **性能**：对于目标为文件的 `http::FileBody` 的 `GET` 请求，底层可通过 `sendfile()` 实现零拷贝 I/O，提高大文件传输效率。
* **配置驱动**：根据 `config.json` 中 `location` 块处理 `root`、`alias`、`index` 等指令。

### `UploadHandler`：文件上传

处理客户端文件上传请求（`POST`/`PUT`）。

* **功能**：从 URI 安全提取文件名，将请求体写入配置的上传目录。
* **响应**：上传成功返回 `201 Created`，并附 JSON 成功信息。

### `RedirectHandler`：HTTP 重定向

专门用于返回重定向响应。

* **使用**：构造函数接收状态码（如 `301`、`302`、`307`）和目标 URL。
* **行为**：忽略请求内容，返回带 `Location` 头的响应。

### `CgiHandler`：动态内容网关

实现 CGI/1.1 规范，通过外部脚本生成动态响应。

* **职责**：接收请求、验证 CGI 脚本路径、解析输出头部和主体，构建标准响应。
* **设计**：进程执行逻辑由 `ICgiRunner` 抽象接口处理，实现解耦。
* **(详细的实现机制，请参见“杂项”章节中的 `CGI (通用网关接口) 详解`)**

### `FunctionHandler`：Lambda 适配器

将普通 C++ 函数或 lambda 包装成 `IRequestHandler`。

* **优势**：可以快速为路由绑定简单逻辑，无需额外类定义。
* **应用**：常用于测试或小型 API 端点，如健康检查接口。

---

## lib/http/routing（路由与请求分发）

本模块负责 **接收 `http::Request` 并分发给合适的处理器**。它是一个轻量、可组合、Header-Only 的路由框架，核心思想基于策略模式和责任链模式。

### `GenericRouter`：通用路由器

基于 **策略模式 (Policy-Based Design)**，通过模板参数定义核心行为：

* **`Key`**：路由匹配的键类型（例如路径字符串或 HTTP 方法）。
* **`KeyFromRequest`**：函数对象，从 `http::Request` 中提取 Key。
* **`Matcher`**：函数对象，根据 Key 容器查找最优匹配项。

这样，路由遍历、匹配和回退逻辑只需在 `GenericRouter` 中实现一次，方便复用。

### `RequestRouter`：责任链分发器

实现 `IRequestDispatcher` 接口，基于 **责任链模式**。

* **内部结构**：维护一个 `IRequestHandler` 管道 `_pipeline`。
* **请求处理流程**：

  1. 按顺序将请求传给管道中每个处理器。
  2. 如果处理器返回非 `404`，立即返回响应。
  3. 如果返回 `404`，尝试下一个处理器。
  4. 全部处理器返回 `404` 时，最终返回 `404 Not Found`。


### 具体路由器（类型别名）

通过为 `GenericRouter` 提供不同策略生成具体路由器：

* **`PrefixRouter`**

  * **Key**：`std::string`（URI）
  * **匹配策略**：Longest Prefix Match（最长前缀匹配）
  * **用途**：实现 Nginx 风格 `location /path/ {...}`

* **`MethodRouter`**

  * **Key**：`http::Method`（GET, POST, …）
  * **匹配策略**：Exact Key Match（精确匹配）
  * **用途**：同一路径下按 HTTP 方法分发请求


### `ExactRouter`：精确匹配

用于直接绑定 `std::function` 而非 `IRequestHandler` 的场景（例如 Lambda 或测试用例）。

* **内部结构**：`_handlers` map 使用 `(Method, Path)` 作为键，`ConcreteHandler` (`std::function`) 作为值。
* **用途**：顶层精确匹配 API 端点或特定页面请求。

### 路由组装 (`RouterBuilder`)

`RouterBuilder` 读取 `config.json` 并组合路由组件：

1. 创建顶层 `RequestRouter` 管道。
2. 遍历每个 `location` 配置块：
   a. 创建 `PrefixRouter`（前缀为 `location.path`）
   b. 在 `PrefixRouter` 内部创建 `MethodRouter`
   c. 根据 `location.methods` 配置添加具体处理器（如 `StaticFileHandler`）
   d. 将配置好的 `PrefixRouter` 加入顶层管道

这种方式实现了完整的 `config.json` 路由逻辑，同时保持 `lib/http/*` 的通用性和可复用性。

---

## 杂项 (Miscellaneous) / 核心机制详解

### Keep-Alive

HTTP/1.1 支持持久连接（Keep-Alive），允许在一个 TCP 连接上处理多个请求-响应周期。 

- `HttpProtocolHandler` 根据请求头和配置判断是否复用连接。  
- `ResponseWriter` 根据最终决策设置 `Connection: keep-alive` 或 `Connection: close`。  
- 当一次请求-响应完成后，如果 Keep-Alive 被允许，则状态机回到 `READING_REQUEST`，否则返回 `Finished`，由 `Connection` 关闭连接。

### Chunked Encoding（分块传输编码）

HTTP/1.1 允许使用 **`Transfer-Encoding: chunked`** 传输长度未知的数据。  

- `RequestParser` 支持完整的 Chunked 编码解析，防止协议攻击和分块拼接攻击。  
- 状态机由 `Step::ChunkedBody` 驱动：

  1. **读取块大小行**：十六进制数，可选扩展，块大小为 `0` 时进入尾部解析。  
  2. **读取块数据**：精确读取前述字节数，数据后必须跟 `CRLF`。  
  3. **读取 Trailer（可选）**：块大小为 0 时，解析额外头部字段，以 `CRLF CRLF` 结束。  

- 健壮性设计：
  * 提前结束检测：数据截断返回 `400 Bad Request`。
  * 大小限制：累积超过 `_max_body_size` 返回 `413 Payload Too Large`。
  * 空行处理：允许块大小行前多余 `CRLF`，兼容部分客户端。
  * 严格语法检查：块大小行中非十六进制字符视为错误。

### CGI (通用网关接口) 

实现了 CGI/1.1 规范，赋予了服务器执行动态脚本的能力

#### 架构分层

CGI 的实现被清晰地划分为两个层次：

1.  **`CgiHandler` (高层协调者)**:
    *   实现了 `IRequestHandler` 接口，是 CGI 功能与路由框架的**连接点**。
    *   它负责将 HTTP `Request` 转换为 CGI 脚本执行所需的上下文，并调用底层的 Runner。
    *   其核心任务是**解析** CGI 脚本的标准输出。它能正确处理 `Status:` 头部（允许脚本自定义 HTTP 状态码），将其他 `Key: Value` 行转换为 HTTP 响应头，并将头部和主体以 `\n\n` 或 `\r\n\r\n` 分隔，最终构建一个可供发送的 `http::Response` 对象。

2.  **`ICgiRunner` / `DefaultCgiRunner` (底层执行者)**:
    *   `CgiHandler` 通过依赖注入的方式，依赖于抽象的 `ICgiRunner` 接口，这使得其单元测试变得极其简单（通过 `MockCgiRunner`）。
    *   `DefaultCgiRunner` 是该接口的**具体实现**，它封装了所有与**操作系统**相关的、复杂的底层操作：
        *   **进程创建**: 使用 `fork()` 创建子进程。
        *   **进程间通信 (IPC)**: 使用 `pipe()` 创建两个管道，分别用于重定向子进程的 `stdin` 和 `stdout`。
        *   **I/O 重定向**: 在子进程中，使用 `dup2()` 将管道的末端连接到 `STDIN_FILENO` 和 `STDOUT_FILENO`。
        *   **环境变量**: 根据 CGI 规范，动态构建一个包含 `REQUEST_METHOD`, `QUERY_STRING`, `CONTENT_LENGTH`, `HTTP_*` 等所有必需元变量的环境变量数组。
        *   **工作目录**: 在执行脚本前，**关键地**使用 `chdir()` 将子进程的当前工作目录切换到 CGI 脚本所在的目录，确保了脚本对本地资源的相对路径访问是正确的。
        *   **脚本执行**: 最终通过 `execve()` 加载并执行指定的 CGI 解释器（如 `/usr/bin/python3`）。

#### 数据流

-   **请求 (Request Body)**: 父进程 (WebServer) 将 `request.body` 中的数据写入 `stdin` 管道的写端。子进程 (CGI 脚本) 从其 `STDIN_FILENO` 读取这些数据。父进程在写完后会关闭写端，向子进程发送 `EOF`。
-   **响应 (Response)**: 子进程将所有输出（HTTP 头部和主体）写入其 `STDOUT_FILENO`。父进程从 `stdout` 管道的读端循环 `read()`，直到遇到 `EOF`，从而捕获完整的 CGI 输出。

---


