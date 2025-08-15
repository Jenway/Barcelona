<!-- doc/reactor-core.md -->
# Reactor 核心模块 (`lib/core`)

本模块实现了服务器事件驱动模型的核心，包括 I/O 多路复用、事件分发、声明式事件绑定，以及连接状态机管理。它是整个网络栈的中枢，负责将底层操作系统事件高效、安全地分发给上层业务逻辑。

---

## 1. `Poller` — 高性能 I/O 事件分发器

> Event Demultiplexer

`Poller` 是对 Linux 高性能 `epoll` 的现代 C++ 封装。它将 OS 层面的事件通知机制抽象为平台无关的 `core::EventType` 枚举（`Read` / `Write` / `None`），屏蔽了底层 `EPOLLIN`、`EPOLLOUT` 等细节。

### 特性

* **高性能**
  使用 `epoll` 的水平触发 (Level-Triggered) 模式，复杂度 O(1)，适合处理高并发连接。

* **简洁 API**

  ```cpp
  addFd(fd)                  // 将 fd 纳入 epoll 监控
  removeFd(fd)               // 从 epoll 中移除 fd
  updateEvents(fd, events)   // 更新 fd 的监听事件
  registerCallback(fd, events, cb) // 为 fd 的特定事件注册回调
  pollOnce(timeout)          // 等待并分发一次事件
  ```

* **现代 C++ 实践**

  * **`std::function`**：所有回调都使用 `std::function<void()>` 存储，支持任意可调用对象（函数指针、Lambda、绑定成员函数等）。
  * **`std::expected`**：所有系统调用封装返回 `std::expected<void, std::system_error>`，强制显式错误处理，避免异常开销。

* **健壮性**

  * 能安全地处理**在回调函数内部移除其自身文件描述符**的场景（通过复制回调列表避免迭代器失效）。
  * 内部处理 `epoll_wait` 被信号中断 (`EINTR`) 的情况，对调用者透明,屏蔽了这些底层细节。

---

## 2. `Channel` — 事件蓝图

`Channel` 是一个与具体 I/O 机制解耦的**事件描述对象**。它只负责声明：

* 哪个 fd 要监听
* 触发读/写事件时调用哪个回调

```cpp
Channel ch(fd);
ch.setReadableHandler([] { /* 读事件逻辑 */ });
ch.setWritableHandler([] { /* 写事件逻辑 */ });
```

`Channel` **不关心** `epoll` 或其他事件机制的存在，它只是事件配置的容器。这种蓝图式设计便于测试、解耦和复用。

---

## 3. `bind_to` — 声明式事件绑定 CPO

> 自定义点对象 (Customization Point Object, CPO)

`bind_to` 是一个基于 **`tag_invoke`** 的自定义点对象（CPO），语义是：

> “将一个 `ChannelLike` 对象绑定到一个 `RegisterLike` 事件引擎上”

当前的默认实现是 `Channel` → `Poller` 绑定：

```cpp
Channel ch(fd);
bind_to(ch, poller);
```

内部步骤：

1. `poller.addFd(fd)` — 将 fd 添加到 epoll。
2. 为 `Read` / `Write` 回调调用 `poller.registerCallback`。
3. 调用 `poller.updateEvents` 设置初始监听事件。

优势：

* 非侵入式：无需改动 `Channel` 或 `Poller` 内部代码就能支持新组合（例如 `Channel` 绑定到 `MockPoller`）。
* 可测试性：可在单元测试中替换为记录绑定行为的 `TestPoller`。

---

## 4. `Connection` — 协调器与状态机

> 事件分发器 (Event Dispatcher) 和状态机 (State Machine)。

`Connection` 负责调度 I/O 与协议处理，不直接进行底层读写。它持有：

* `Socket` — 资源句柄
* `ISource` — 抽象的读数据源，封装了所有底层的**读 I/O** 逻辑（如 `TcpSource`）。
* `ISinker` — 抽象的写数据汇，封装了所有底层的**写 I/O** 逻辑（如 `TcpSinker`）。
* `IHandler` — 协议处理器（如 HTTP），它的具体实现（如 `HttpProtocolHandler`）封装了所有的应用层协议逻辑。

### 核心事件回调

* **`onReadable()`**
  读取数据 → 交给 `IHandler` → 根据协议状态更新连接状态
* **`onWritable()`**
  让 `IHandler` 使用 `ISinker` 写数据 → 更新状态

---

## 5. 连接状态机与优雅关闭

`Connection` 使用四态模型：

* `READING` — 监听读事件
* `WRITING` — 监听写事件
* `CLOSING` — 已半关闭写端，等待对方 `FIN`
* `CLOSED` — 完全关闭，等待析构释放 fd

`Connection` 根据 `IHandler` 返回的状态（`WantRead`, `WantWrite`, `Finished` 等）来更新自己的内部状态（`READING`, `WRITING`, `CLOSING`），并以此通过 `interestedEvents()` 方法，告诉 `Server` 下一步应该向 `Poller` 注册什么事件。

### 优雅关闭流程

1. 当 `IHandler` 返回 `Finished` 状态 → 进入 `CLOSING`
2. 调用 `socket.shutdownWrite()` 发送 FIN
3. 继续监听 `Read`，丢弃接收数据直到 `EOF`
4. 收到 `EOF` → 进入 `CLOSED` → 释放资源

-   **半关闭 (`shutdown`)**: 当协议处理器（`IHandler`）指示连接应关闭时，`Connection` 不会立即调用 `close()`，而是先调用 `socket.shutdownWrite()`。这会向客户端发送一个 `FIN` 报文，明确告知对方服务器不会再发送任何数据。
-   **`CLOSING` 状态**: 发送 `FIN` 后，`Connection` 会进入一个新的 `CLOSING` 状态。在此状态下，它会继续监听读事件，以“排干”客户端可能仍在发送的数据，并等待接收客户端的 `FIN` 报文（表现为 `read()` 返回 `EOF`）。
-   **资源回收**: 只有在成功接收到客户端的 `EOF`、确认四次挥手基本完成后，`Connection` 才会最终进入 `CLOSED` 状态，并允许其底层的 `FileDescriptor` 通过 RAII 机制安全地调用 `close()` 进行资源回收。

好处：

* 确保客户端收到完整响应（解决 413 等场景下响应丢失）
* 正确完成 TCP 四次挥手，避免 TIME\_WAIT 或连接重置


---

## 6. Reactor 主循环示例

```cpp
Poller poller;

// 创建 Channel 并绑定回调
Channel ch(conn.fd());
ch.setReadableHandler([&] { conn.onReadable(); });
ch.setWritableHandler([&] { conn.onWritable(); });
bind_to(ch, poller);

// 事件循环
while (running) {
    poller.pollOnce(-1);
}
```

此循环中，`Poller` 负责感知事件并调用 `Channel` 的回调，`Connection` 驱动状态机，`IHandler` 执行业务逻辑。

---

## 7. 架构优势总结

* **分层解耦**：`core` 层不直接依赖 HTTP、CGI 等业务协议
* **平台无关**：`EventType` 抽象屏蔽了底层 OS API
* **声明式注册**：`Channel` + `bind_to` 替代命令式事件注册
* **健壮关闭**：完整支持优雅的 TCP 终止流程
* **可测试性强**：任何层都可用 Mock 替换，独立测试

___
