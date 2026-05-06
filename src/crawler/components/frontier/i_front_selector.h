#pragma once

#include <concepts>
#include <vector>

#include "multiqueue_containers.h"
#include "types/url.h"

namespace crawler {

namespace components {

using StatelessFrontSelector = std::tuple<>;

class IFrontSelector {
 public:
  using FrontQueueContainer =
      MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<types::URL>>;

  virtual std::optional<types::URL> extract(
      FrontQueueContainer& frontQueues) = 0;
  virtual std::vector<types::URL> extractBatch(FrontQueueContainer& frontQueues,
                                               std::size_t maxCount) = 0;
  virtual ~IFrontSelector() = default;
};

using FrontQueueContainer =
    MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<types::URL>>;

template <typename T>
concept FrontSelectorType =
    requires(FrontQueueContainer& fqueues, std::size_t maxCount) {
      {
        std::declval<T>().extract(fqueues)
      } -> std::same_as<std::optional<types::URL>>;
      {
        std::declval<T>().extractBatch(fqueues, maxCount)
      } -> std::same_as<std::vector<types::URL>>;
    };

}  // namespace components

}  // namespace crawler
