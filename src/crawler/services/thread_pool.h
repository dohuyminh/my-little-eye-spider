#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace crawler {

namespace services {

namespace concurrency {

class ThreadPool {
 public:
  ThreadPool(size_t numThreads);

  bool isBusy() const noexcept {
    std::unique_lock<std::mutex> lock(queueMutex_);
    return !tasks_.empty();
  }

  void start() noexcept;
  void stop() noexcept;
  void threadLoop() noexcept;

  template <typename Fn = void, typename... Args>
  auto enqueue(Fn&& fn, Args&&... args) -> std::future<decltype(fn(args...))> {
    using ReturnType = decltype(fn(args...));

    auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
        [fn = std::forward<Fn>(fn),
         ... args = std::forward<Args>(args)]() mutable {
          return std::invoke(std::move(fn), std::move(args)...);
        });

    auto future = taskPtr->get_future();

    // Single lambda wrapper
    tasks_.push([taskPtr]() { (*taskPtr)(); });
    condition_.notify_one();

    return future;
  }

 private:
  size_t numThreads_{0};
  std::vector<std::thread> workers_;
  std::queue<std::move_only_function<void()>> tasks_;

  mutable std::mutex queueMutex_;
  std::condition_variable condition_;
  bool stop_{true};

 public:
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
};

}  // namespace concurrency

}  // namespace services

}  // namespace crawler