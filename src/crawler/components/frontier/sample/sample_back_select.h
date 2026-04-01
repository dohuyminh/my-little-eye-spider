#pragma once

#include "../i_back_selector.h"

namespace crawler {

namespace components {

class SampleBackSelector : public IBackSelector {
 public:
  std::optional<types::URL> extract(IBackSelector::BackQueueContainer& backQueues) override;
  std::vector<types::URL> extractBatch(IBackSelector::BackQueueContainer& backQueues,
                                       std::size_t maxCount) override;
};

}  // namespace components

}  // namespace crawler