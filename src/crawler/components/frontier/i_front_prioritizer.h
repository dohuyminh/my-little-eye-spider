#pragma once

#include <concepts>
#include <utility>

#include "types/url.h"
#include "valid_queue_return.h"

namespace crawler {

namespace components {

using StatelessFrontPrioritizer = std::tuple<>;

class IFrontPrioritizer {
 public:
  virtual std::pair<types::URL, std::size_t> selectQueue(
      const std::string& url) = 0;
  virtual ~IFrontPrioritizer() = default;
};

template <typename T, typename QDT>
concept FrontPrioritizerType =
    ValidQueueDataType<QDT> && requires(const std::string& url) {
      {
        std::declval<T>().selectQueue(url)
      } -> std::same_as<std::pair<QDT, std::vector<std::size_t>>>;
    };

}  // namespace components

}  // namespace crawler
