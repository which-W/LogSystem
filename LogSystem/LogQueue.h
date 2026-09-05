#pragma once
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <string>
namespace logsystem {
class LogQueue {
public:
    explicit LogQueue(std::size_t capacity);
    bool push(std::string message, std::chrono::milliseconds wait_for_space);
    bool pop(std::string& message);
    void shutdown();
private:
    const std::size_t capacity_;
    std::queue<std::string> queue_;
    std::mutex mutex_;
    std::condition_variable readable_, writable_;
    bool shutdown_ = false;
};
} // namespace logsystem
