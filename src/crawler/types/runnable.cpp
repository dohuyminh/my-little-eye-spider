#include "runnable.h"

namespace crawler {

namespace types {

void Runnable::run() {
    
    // Atomic check-and-set: only run if not already running
    bool expected = false;
    if (!isRunning_.compare_exchange_strong(expected, true)) {
        return;  // Already running
    }

    // Create thread to run implementation
    eventLoopThread_ = std::thread([this]() {
        while (isRunning_.load()) {
            runImpl();
            
            // Small sleep to prevent busy-waiting and allow graceful shutdown
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
}

void Runnable::stop() noexcept {
    isRunning_.store(false);
    
    if (eventLoopThread_.joinable()) {
        eventLoopThread_.join();
    }
}

}

}