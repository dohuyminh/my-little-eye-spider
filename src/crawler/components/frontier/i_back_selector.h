#pragma once

#include <vector>

#include "multiqueue_containers.h"
#include "types/url.h"
#include "valid_queue_return.h"

namespace crawler {

namespace components {

using StatelessBackSelector = std::tuple<>;

class IBackSelector {
 public:
  using BackQueueContainer = MultiQueueContainers<StdQueueWrapper<types::URL>>;

  virtual std::optional<types::URL> extract(BackQueueContainer& backQueues) = 0;
  virtual std::vector<types::URL> extractBatch(BackQueueContainer& backQueues,
                                               std::size_t maxCount) = 0;
  virtual ~IBackSelector() = default;
};

template <typename QDT>
using BackQueueContainer = MultiQueueContainers<StdQueueWrapper<QDT>>;

template <typename T, typename QDT>
concept BackSelectorType =
    ValidQueueDataType<QDT> &&
    requires(BackQueueContainer<QDT>& bqueues, std::size_t maxCount) {
      {
        std::declval<T>().extract(bqueues)
      } -> std::same_as<std::optional<QDT>>;
      {
        std::declval<T>().extractBatch(bqueues, maxCount)
      } -> std::same_as<std::vector<QDT>>;
    };

}  // namespace components

}  // namespace crawler
