#pragma once

#include "../i_front_selector.h"

namespace crawler {

namespace components {

class SampleFrontSelector : public IFrontSelector {
 public:
  std::optional<types::URL> extract(IFrontSelector::FrontQueueContainer& frontQueues) override;
  std::vector<types::URL> extractBatch(IFrontSelector::FrontQueueContainer& frontQueues,
                                       std::size_t maxCount) override;
};

}  // namespace components

}  // namespace crawler