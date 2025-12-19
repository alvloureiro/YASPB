#pragma once

#include <thread>

namespace playback {

// RAII Thread Wrapper
// Ensures thread is always properly joined
// This utility class provides exception-safe thread management with automatic
// cleanup. It's useful for managing threads that need to be restarted or
// properly cleaned up when objects are destroyed.
class ThreadGuard {
   public:
    ThreadGuard() = default;

    ~ThreadGuard() noexcept {
        // Destructors should not throw, so we catch any exceptions from join()
        // This ensures exception safety even if join() throws
        try {
            if (thread_.joinable()) {
                thread_.join();
            }
        } catch (...) {
            // Thread join failed - this is unusual but we can't throw from destructor
            // In most cases, this indicates a serious error, but we must handle it gracefully
            // The thread will be detached implicitly when thread_ is destroyed
        }
    }

    // Non-copyable
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;

    // Movable
    ThreadGuard(ThreadGuard&& other) noexcept : thread_(std::move(other.thread_)) {}
    ThreadGuard& operator=(ThreadGuard&& other) noexcept {
        if (this != &other) {
            // Join current thread before moving - use try-catch for exception safety
            try {
                if (thread_.joinable()) {
                    thread_.join();
                }
            } catch (...) {
                // If join fails, we still proceed with the move
                // The old thread will be lost, but this is better than throwing
            }
            thread_ = std::move(other.thread_);
        }
        return *this;
    }

    // Start or restart a thread with the given function and arguments.
    // If a thread is already running, it will be joined before starting a new one.
    template <typename Function, typename... Args>
    void start(Function&& f, Args&&... args) {
        // Ensure previous thread is joined before starting new one
        if (thread_.joinable()) {
            thread_.join();
        }
        thread_ = std::thread(std::forward<Function>(f), std::forward<Args>(args)...);
    }

    [[nodiscard]] bool joinable() const {
        return thread_.joinable();
    }

    void join() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    [[nodiscard]] std::thread::id get_id() const {
        return thread_.get_id();
    }

   private:
    std::thread thread_;
};

}  // namespace playback
