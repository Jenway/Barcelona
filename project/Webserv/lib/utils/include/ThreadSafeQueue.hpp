// in lib/utils/include/ThreadSafeQueue.hpp
#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    /**
     * @brief 向队列中推入一个元素 (生产者调用)
     * @param value 要推入的元素
     */
    void push(T value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(std::move(value));
        _cond_var.notify_one();
    }

    /**
     * @brief 尝试从队列中弹出一个元素 (消费者调用, 非阻塞)
     * @return 如果队列不为空，则返回元素；否则返回 std::nullopt
     */
    auto try_pop() -> std::optional<T>
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            return std::nullopt;
        }
        T value = std::move(_queue.front());
        _queue.pop();
        return value;
    }

    /**
     * @brief 等待并从队列中弹出一个元素 (消费者调用, 阻塞)
     * @param value_ref 用于接收弹出元素的引用
     */
    void wait_and_pop(T& value_ref)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond_var.wait(lock, [this] { return !_queue.empty(); });
        value_ref = std::move(_queue.front());
        _queue.pop();
    }

    [[nodiscard]] bool empty() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }

private:
    mutable std::mutex _mutex;
    std::queue<T> _queue;
    std::condition_variable _cond_var;
};