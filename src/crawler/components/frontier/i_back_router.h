#pragma once

#include "types/url.h"

namespace crawler {

namespace components {

using StatelessBackRouter = std::tuple<>;

class IBackRouter {
 public:
  virtual std::size_t routeURL(const types::URL& url) = 0;
  virtual ~IBackRouter() = default;
};

template <typename T>
concept BackRouterType = requires(const types::URL& url) {
  { std::declval<T>().routeURL(url) } -> std::same_as<std::size_t>;
};

}  // namespace components

}  // namespace crawler
