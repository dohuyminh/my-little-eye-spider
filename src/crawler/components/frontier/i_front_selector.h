#pragma once

#include <vector>

#include "front_queues.h"

namespace crawler {

namespace components {

class IFrontSelector {
 public:
  virtual std::optional<types::URL> extract(FrontQueues& frontQueues) = 0;
  virtual std::vector<types::URL> extractBatch(FrontQueues& frontQueues,
                                               std::size_t maxCount) = 0;
  virtual ~IFrontSelector() = default;
};

}  // namespace components

}  // namespace crawler