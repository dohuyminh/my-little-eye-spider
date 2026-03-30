#include "coroutine_thread_pool.h"

namespace crawler {

namespace services {

namespace concurrency {

CoroutineThreadPool::CoroutineThreadPool(std::size_t numThreads) : 
  workGuard_(ioCtx_.get_executor())
{
  if (numThreads == 0) {
    numThreads = std::thread::hardware_concurrency();
  }

  threads_.reserve(numThreads);

  for (std::size_t i{0}; i < numThreads; ++i) {
    threads_.emplace_back([this]() {
      ioCtx_.run();
    });
  }
}

CoroutineThreadPool::~CoroutineThreadPool() {
  workGuard_.reset();
  for (auto& thread: threads_) {
    thread.join();
  }
}

}

}

}