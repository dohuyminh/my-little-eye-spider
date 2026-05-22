#pragma once

#include <concepts>

#include "moodycamel/concurrentqueue.h"
#include "services/thread_pool.h"
#include "types/runnable.h"
#include "worker.h"

namespace crawler {

namespace components {

template <FrontierType F, typename WorkerType>
  requires std::derived_from<WorkerType, Worker<F>>
class WorkerPool : public types::Runnable {
 public:
  WorkerPool(std::size_t numWorkers, std::size_t batchSize,
             const WorkerType& workerBluepint)
      : threadPool_(numWorkers),
        batchSize_(batchSize),
        workerBlueprint_(workerBluepint) {
    threadPool_.start();
  }

  void preLoop() override { threadPool_.start(); }

  void runImpl() override {
    // extract URLs in batch
    std::vector<typename F::DataType> batch;
    batch.reserve(batchSize_);

    consumingURLs_->try_dequeue_bulk(std::back_inserter(batch), batchSize_);

    // for each extracted URL, submit a job to worker
    for (auto& url : batch) {
      auto worker = std::make_unique<WorkerType>(workerBlueprint_);
      worker->assignData(std::move(url));
      threadPool_.enqueue(
          [worker = std::move(worker)]() mutable { worker->doWork(); });
    }
  }

  void stop() override {
    threadPool_.stop();
    Runnable::stop();
  }

 private:
  // continuously consume URLs from queue
  std::shared_ptr<moodycamel::ConcurrentQueue<typename F::DataType>>
      consumingURLs_;

  // produce new URLs for the frontier to consume
  std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs_;

  // assign each URL to a working thread among the pool
  services::concurrency::ThreadPool threadPool_;

  // at each iteration, an URL batch of size b_b is extracted
  std::size_t batchSize_;

  // Worker's blueprint to be copied
  WorkerType workerBlueprint_;
};

}  // namespace components

}  // namespace crawler
