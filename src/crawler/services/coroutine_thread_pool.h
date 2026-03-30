#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <vector>
#include <thread>

namespace crawler {

namespace services {

namespace concurrency {

class CoroutineThreadPool {
public:
  explicit CoroutineThreadPool(std::size_t numThreads = 0);

  template <typename AwaitableFn, typename ...Args>
  auto enqueue(AwaitableFn&& fn, Args&& ...args) {
    using Awaitable = std::invoke_result_t<AwaitableFn, Args...>;
    using ReturnType = typename Awaitable::value_type;

    // Start the coroutine on the io_context and obtain a future for its result
    return boost::asio::co_spawn(
      ioCtx_,
      std::invoke(std::forward<AwaitableFn>(fn), std::forward<Args>(args)...),
      boost::asio::use_future
    );
  }

  ~CoroutineThreadPool();

private:
  boost::asio::io_context ioCtx_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard_;
  std::vector<std::thread> threads_;
};

}

}

}