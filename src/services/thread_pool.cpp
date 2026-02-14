#include "thread_pool.h"

namespace services {

namespace concurrency {

ThreadPool::ThreadPool(size_t numThreads) : numThreads_(numThreads) {}

void ThreadPool::start() noexcept {
    stop_ = false;
    for (size_t i = 0; i < numThreads_; ++i) {
        workers_.emplace_back(&ThreadPool::threadLoop, this);
    }
}

void ThreadPool::stop() noexcept {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers_.clear();
}

void ThreadPool::threadLoop() noexcept {
    while (true) {
        std::move_only_function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty()) {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}

}

}