#pragma once

#include "../i_back_selector.h"

namespace crawler {

namespace components {

class RoundRobinBackSelector : public IBackSelector {
 public:
  RoundRobinBackSelector() = default;

  std::optional<types::URL> extract(IBackSelector::BackQueueContainer& backQueues) override;
  std::vector<types::URL> extractBatch(IBackSelector::BackQueueContainer& backQueues,
                                       std::size_t maxCount) override;

 private:
  std::size_t pointer_{0};

  std::mutex mutex_;
};

}  // namespace components

}  // namespace crawler