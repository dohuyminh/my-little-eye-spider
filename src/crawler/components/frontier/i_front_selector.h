#pragma once

#include <vector>

#include "multiqueue_containers.h"
#include "types/url.h"

namespace crawler {

namespace components {

class IFrontSelector {
 public:
  using FrontQueueContainer = MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<types::URL>>;

  virtual std::optional<types::URL> extract(FrontQueueContainer& frontQueues) = 0;
  virtual std::vector<types::URL> extractBatch(FrontQueueContainer& frontQueues,
                                               std::size_t maxCount) = 0;
  virtual ~IFrontSelector() = default;
};

}  // namespace components

}  // namespace crawler