#pragma once

#include "types/url.h"
#include "valid_queue_return.h"

namespace crawler {

namespace components {

using StatelessBackRouter = std::tuple<>;

class IBackRouter {
 public:
  virtual std::size_t routeURL(const types::URL& url) = 0;
  virtual ~IBackRouter() = default;
};

template <typename T, typename QDT>
concept BackRouterType = ValidQueueDataType<QDT> && requires(const QDT& url) {
  { std::declval<T>().routeURL(url) } -> std::same_as<std::size_t>;
};

}  // namespace components

}  // namespace crawler
