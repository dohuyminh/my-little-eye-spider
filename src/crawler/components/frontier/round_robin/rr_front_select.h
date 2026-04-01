#pragma once

#include <mutex>

#include "../i_front_selector.h"

namespace crawler {

namespace components {

class RoundRobinFrontSelector : public IFrontSelector {
 public:
  RoundRobinFrontSelector() = default;

  std::optional<types::URL> extract(IFrontSelector::FrontQueueContainer& frontQueues) override;
  std::vector<types::URL> extractBatch(IFrontSelector::FrontQueueContainer& frontQueues,
                                       std::size_t maxCount) override;

 private:
  std::size_t pointer_{0};

  std::mutex mutex_;
};

}  // namespace components

}  // namespace crawler
