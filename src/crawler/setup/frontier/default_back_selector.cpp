#include "default_back_selector.h"

#include "politeness_tracker.h"

namespace crawler::setup {

std::optional<types::URL> DefaultBackSelector::extract(
    BackQueueContainer& containers) {
  // Implementation for extracting URL from the back of the queue
  // Example: Round-robin style for extracting URLs
  for (std::size_t i{ptr_}; i == ptr_ || i % ptr_ > 0; ++i) {
    auto elem{containers.dequeue(i % containers.numQueues())};
    if (elem) {
      ptr_ = (ptr_ + 1) % containers.numQueues();

      // check if domain is allowed/ready to be crawled
      if (PolitenessTracker::get().handleURL(elem.value()) ==
          DomainStatus::ALLOWED) {
        return elem;
      }
    }
  }

  return std::nullopt;
}

std::vector<types::URL> DefaultBackSelector::extractBatch(
    BackQueueContainer& containers, size_t max_urls) {
  // Implementation for extracting a batch of URLs from the back of the queue
  std::vector<types::URL> batch;
  for (size_t i = 0; i < max_urls; ++i) {
    auto url = extract(containers);
    if (!url) {
      break;
    }
    batch.push_back(*url);
  }
  return batch;
}

}  // namespace crawler::setup
