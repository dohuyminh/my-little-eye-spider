#pragma once

#include <vector>

#include "multiqueue_containers.h"
#include "types/url.h"

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

using BackQueueContainer = MultiQueueContainers<StdQueueWrapper<types::URL>>;

template <typename T>
concept BackSelectorType =
    requires(BackQueueContainer& bqueues, std::size_t maxCount) {
      {
        std::declval<T>().extract(bqueues)
      } -> std::same_as<std::optional<types::URL>>;
      {
        std::declval<T>().extractBatch(bqueues, maxCount)
      } -> std::same_as<std::vector<types::URL>>;
    };

}  // namespace components

}  // namespace crawler
