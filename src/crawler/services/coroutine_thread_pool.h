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

  template <bool ReturnFuture = true, typename AwaitableFn, typename ...Args>
  auto enqueue(AwaitableFn&& fn, Args&& ...args) {
    // Capture fn and args as a tuple to preserve their values
    // and give co_spawn full control over the awaitable lifetime
    auto task = [fn = std::forward<AwaitableFn>(fn),
                 args_tuple = std::tuple{std::forward<Args>(args)...}]
                () mutable -> boost::asio::awaitable<void> {
      co_return co_await std::apply(std::forward<AwaitableFn>(fn), args_tuple);
    };

    if constexpr (ReturnFuture) {
      return boost::asio::co_spawn(
        ioCtx_,
        std::move(task),
        boost::asio::use_future
      );
    } else {
      boost::asio::co_spawn(
        ioCtx_,
        std::move(task),
        boost::asio::detached
      );
    }
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