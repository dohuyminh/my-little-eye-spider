#pragma once

#include <concepts>
#include <vector>

#include "multiqueue_containers.h"
#include "types/url.h"
#include "valid_queue_return.h"

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

template <typename T, typename QDT>
concept FrontSelectorType =
    ValidQueueDataType<QDT> &&
    requires(FrontQueueContainer& fqueues, std::size_t maxCount) {
      {
        std::declval<T>().extract(fqueues)
      } -> std::same_as<std::optional<QDT>>;
      {
        std::declval<T>().extractBatch(fqueues, maxCount)
      } -> std::same_as<std::vector<QDT>>;
    };

}  // namespace components

}  // namespace crawler
