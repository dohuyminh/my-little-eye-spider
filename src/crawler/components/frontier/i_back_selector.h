#pragma once

#include <vector>

#include "multiqueue_containers.h"
#include "types/url.h"

namespace crawler {

namespace components {

class IBackSelector {
 public:
  using BackQueueContainer = MultiQueueContainers<StdQueueWrapper<types::URL>>;

  virtual std::optional<types::URL> extract(BackQueueContainer& backQueues) = 0;
  virtual std::vector<types::URL> extractBatch(BackQueueContainer& backQueues,
                                               std::size_t maxCount) = 0;
  virtual ~IBackSelector() = default;
};

}  // namespace components

}  // namespace crawler