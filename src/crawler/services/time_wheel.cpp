#include "time_wheel.h"

namespace crawler {

namespace services {

namespace concurrency {

Task::Task(std::move_only_function<void()> executable,
           std::chrono::seconds delay)
    : executable_(std::move(executable)), delay_(delay) {}

void TimerWheel::runImpl() {
  // sleep for tick
  std::this_thread::sleep_for(tickDuration_);

  // iterate through all current tasks and reduce delay time
  std::vector<Task> runningTask;
  Task t;

  while (buckets_[current_].try_dequeue(t)) {
    runningTask.push_back(std::move(t));
  }

  for (auto& task : runningTask) {
    task.delay_ -= tickDuration_;
    if (task.delay_.count() <= 0) {
      task.executable_();
    } else {
      // compute how many ticks are still needed and allocate to appropriate
      // bucket
      std::size_t hash{hash_(task.delay_, buckets_.size())};
      buckets_[hash].enqueue(std::move(task));
    }
  }

  // move on to the next bucket
  current_ = (current_ + 1) % buckets_.size();
}

}  // namespace concurrency

}  // namespace services

}  // namespace crawler