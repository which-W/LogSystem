#include "LogQueue.h"
#include <stdexcept>
namespace logsystem {
LogQueue::LogQueue(std::size_t capacity) : capacity_(capacity) {
    if (capacity_ == 0) throw std::invalid_argument("log queue capacity must be positive");
}
bool LogQueue::push(std::string message, std::chrono::milliseconds wait_for_space) {
    std::unique_lock<std::mutex> lock(mutex_);
    const auto writable = [this] { return shutdown_ || queue_.size() < capacity_; };
    if (wait_for_space.count() == 0) { if (!writable()) return false; }
    else if (!writable_.wait_for(lock, wait_for_space, writable)) return false;
    if (shutdown_) return false;
    queue_.push(std::move(message)); readable_.notify_one(); return true;
}
bool LogQueue::pop(std::string& message) {
    std::unique_lock<std::mutex> lock(mutex_);
    readable_.wait(lock, [this] { return shutdown_ || !queue_.empty(); });
    if (queue_.empty()) return false;
    message = std::move(queue_.front()); queue_.pop(); writable_.notify_one(); return true;
}
void LogQueue::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_); shutdown_ = true;
    readable_.notify_all(); writable_.notify_all();
}
} // namespace logsystem
