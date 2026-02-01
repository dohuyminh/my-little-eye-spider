#include "runnable.h"

namespace crawler {

namespace type {

void Runnable::run() {
    // mark as running
    {
        std::unique_lock<std::mutex> lock(runMutex_);
        isRunning_ = true;
    }

    // create thread to run implementation
    eventLoopThread_ = std::thread([this]() {
        while (true) {
            runImpl();

            // check if should continue running
            {
                std::unique_lock<std::mutex> lock(runMutex_);
                if (!isRunning_) {
                    break;
                }
            }
        }
    });
}

void Runnable::stop() noexcept {
    {
        std::unique_lock<std::mutex> lock(runMutex_);
        isRunning_ = false;
    }
    runCv_.notify_all();
    eventLoopThread_.join();
}

}

}