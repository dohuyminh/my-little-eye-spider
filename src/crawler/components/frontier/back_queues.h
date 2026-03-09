#pragma once

#include <moodycamel/concurrentqueue.h>

#include <format>
#include <optional>
#include <stdexcept>
#include <vector>

#include "types/url.h"

namespace crawler {

namespace components {

class BackQueues {
 public:
  BackQueues(std::size_t numQueues);

  /**
   * Lock-free dequeue from specified bucket.
   * Thread-safe: Multiple threads can call simultaneously.
   *
   * @param queueIndex The bucket to dequeue from
   * @return URL if available, empty optional if bucket empty
   */
  std::optional<types::URL> selectAndPop(std::size_t queueIndex);

  /**
   * Lock-free enqueue to specified bucket.
   * Thread-safe: Multiple threads can enqueue simultaneously (MPMC).
   *
   * @param url The URL to insert
   * @param queueIndex The bucket to insert into
   */
  void insert(const types::URL& url, std::size_t queueIndex);
  /**
   * Lock-free move enqueue to specified bucket.
   * Optimized: Uses move semantics to avoid copies.
   *
   * @param url The URL to insert (moved)
   * @param queueIndex The bucket to insert into
   */
  void insert(types::URL&& url, std::size_t queueIndex);

  std::size_t numQueues() const noexcept { return backQueues_.size(); }

  std::size_t numURLsInQueue(std::size_t queueIndex) const {
    if (queueIndex >= backQueues_.size()) {
      throw std::out_of_range(
          std::format("Indexing queue of order {} from a set of {} queues",
                      queueIndex, backQueues_.size()));
    }

    // Note: Lock-free queues don't provide accurate size (would require lock)
    // This is intentional for performance - use only if necessary
    return backQueues_[queueIndex].data.size_approx();
  }

 private:
  /**
   * Lock-free MPMC queue per bucket.
   * - Multiple producers (crawler threads inserting discovered URLs)
   * - Multiple consumers (back queue processor threads)
   * - Zero-contention: No mutexes, uses atomic operations internally
   */
  struct DomainBucket {
    moodycamel::ConcurrentQueue<types::URL> data;
  };

  std::vector<DomainBucket> backQueues_;
};

}  // namespace components

}  // namespace crawler