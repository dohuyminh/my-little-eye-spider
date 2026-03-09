#include "worker_pool.h"

namespace crawler {

namespace components {

WorkerPool::WorkerPool(
    std::size_t numWorkers,
    std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingURLs,
    std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs,
    std::size_t batchSize)
    : threadPool_(numWorkers),
      consumingURLs_(consumingURLs),
      producingURLs_(producingURLs),
      batchSize_(batchSize) {
  threadPool_.start();
}

void WorkerPool::preLoop() { threadPool_.start(); }

void WorkerPool::runImpl() {
  // extract URLs in batch
  std::vector<types::URL> batch;
  batch.reserve(batchSize_);

  consumingURLs_->try_dequeue_bulk(std::back_inserter(batch), batchSize_);

  // for each extracted URL, submit a job to worker
  for (auto& url : batch) {
    auto worker = std::make_unique<Worker>(producingURLs_, std::move(url));
    threadPool_.enqueue(
        [worker = std::move(worker)]() mutable { worker->doWork(); });
  }
}

void WorkerPool::stop() {
  threadPool_.stop();
  Runnable::stop();
}

}  // namespace components

}  // namespace crawler