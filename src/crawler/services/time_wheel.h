#pragma once

#include <moodycamel/concurrentqueue.h>

#include <atomic>
#include <functional>
#include <future>
#include <vector>

#include "types/runnable.h"

namespace crawler {

namespace services {

namespace concurrency {

struct Task {
  std::move_only_function<void()> executable_;
  std::chrono::seconds delay_;

  Task() = default;

  Task(std::move_only_function<void()> executable, std::chrono::seconds delay);
};

class TimerWheel : public types::Runnable {
 public:
  TimerWheel(std::size_t capacity, int tickDurationSeconds)
      : TimerWheel(capacity, tickDurationSeconds,
                   [](std::chrono::seconds delay, std::size_t bucketCount) {
                     return static_cast<std::size_t>(delay.count()) %
                            bucketCount;
                   }) {}

  template <typename Callable>
  TimerWheel(std::size_t capacity, int tickDurationSeconds, Callable&& hash)
      : buckets_(capacity),
        tickDuration_(std::chrono::seconds(tickDurationSeconds)),
        hash_(std::forward<Callable>(hash)) {}

  template <typename Fn, typename... Args>
  auto delayExecute(std::chrono::seconds delay, Fn&& fn,
                    Args&&... args) -> std::future<decltype(fn(args...))> {
    using ReturnType = decltype(fn(args...));

    auto taskPtr{std::make_shared<std::packaged_task<ReturnType()>>(
        [fn = std::forward<Fn>(fn),
         ... args = std::forward<Args>(args)]() mutable {
          return std::invoke(std::move(fn), std::move(args)...);
        })};

    auto future{taskPtr->get_future()};

    std::size_t hash{hash_(delay, buckets_.size())};

    buckets_[hash].enqueue(Task([taskPtr]() { (*taskPtr)(); }, delay));

    return future;
  }

  friend class TimerWheelTest;

 private:
  void runImpl() override;
  std::vector<moodycamel::ConcurrentQueue<Task>> buckets_;
  std::chrono::seconds tickDuration_;

  std::function<std::size_t(std::chrono::seconds, std::size_t)> hash_;

  std::size_t current_{0};
};

}  // namespace concurrency

}  // namespace services

}  // namespace crawler