#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace services {

namespace pattern {

template <typename T>
class SharedQueue {
public:
    
    SharedQueue() = default;

    template <typename U>
    void push(U&& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.emplace(std::forward<U>(item));
        notEmptyCV_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        notEmptyCV_.wait(lock, [this]() { return !queue_.empty(); });
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }
    
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable notEmptyCV_;
};

}

}