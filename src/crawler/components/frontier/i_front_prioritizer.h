#pragma once

#include <concepts>
#include <utility>

#include "types/url.h"

namespace crawler {

namespace components {

using StatelessFrontPrioritizer = std::tuple<>;

class IFrontPrioritizer {
 public:
  virtual std::pair<types::URL, std::size_t> selectQueue(
      const std::string& url) = 0;
  virtual ~IFrontPrioritizer() = default;
};

template <typename T>
concept FrontPrioritizerType = requires(const std::string& url) {
  {
    std::declval<T>().selectQueue(url)
  } -> std::same_as<std::pair<types::URL, std::vector<std::size_t>>>;
};

}  // namespace components

}  // namespace crawler
