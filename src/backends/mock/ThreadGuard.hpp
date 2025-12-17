#pragma once

#include <thread>

namespace playback {

// RAII Thread Wrapper
// Ensures thread is always properly joined
class ThreadGuard {
   public:
    ThreadGuard() = default;

    ~ThreadGuard() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    // Non-copyable
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;

    // Movable
    ThreadGuard(ThreadGuard&& other) noexcept : thread_(std::move(other.thread_)) {}
    ThreadGuard& operator=(ThreadGuard&& other) noexcept {
        if (this != &other) {
            if (thread_.joinable()) {
                thread_.join();
            }
            thread_ = std::move(other.thread_);
        }
        return *this;
    }

    template <typename Function, typename... Args>
    void start(Function&& f, Args&&... args) {
        // Ensure previous thread is joined before starting new one
        if (thread_.joinable()) {
            thread_.join();
        }
        thread_ = std::thread(std::forward<Function>(f), std::forward<Args>(args)...);
    }

    bool joinable() const {
        return thread_.joinable();
    }

    void join() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    std::thread::id get_id() const {
        return thread_.get_id();
    }

   private:
    std::thread thread_;
};

}  // namespace playback
