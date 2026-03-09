#pragma once

#include "moodycamel/concurrentqueue.h"
#include "services/thread_pool.h"
#include "types/runnable.h"
#include "worker.h"

namespace crawler {

namespace components {

class WorkerPool : public types::Runnable {
 public:
  WorkerPool(
      std::size_t numWorkers,
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingURLs,
      std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs,
      std::size_t batchSize);

  void preLoop() override;

  void runImpl() override;

  void stop() override;

 private:
  // continuously consume URLs from queue
  std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingURLs_;

  // produce new URLs for the frontier to consume
  std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs_;

  // assign each URL to a working thread among the pool
  services::concurrency::ThreadPool threadPool_;

  // at each iteration, an URL batch of size b_b is extracted
  std::size_t batchSize_;
};

}  // namespace components

}  // namespace crawler