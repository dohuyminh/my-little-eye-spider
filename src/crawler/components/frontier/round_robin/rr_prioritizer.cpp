#include "rr_prioritizer.h"

namespace crawler {

namespace components {

RoundRobinPrioritizer::RoundRobinPrioritizer(std::size_t numQueues)
    : numQueues_(numQueues) {}

std::pair<types::URL, std::size_t> RoundRobinPrioritizer::selectQueue(
    const std::string& url) {
  std::size_t selectedQueue;

  {
    std::unique_lock<std::mutex> lock(mutex_);
    selectedQueue = pointer_;
    pointer_ = (pointer_ + 1) % numQueues_;
  }

  return {types::URL(url), selectedQueue};
}

}  // namespace components

}  // namespace crawler
