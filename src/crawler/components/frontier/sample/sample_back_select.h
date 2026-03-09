#pragma once

#include "../i_back_selector.h"

namespace crawler {

namespace components {

class SampleBackSelector : public IBackSelector {
 public:
  std::optional<types::URL> extract(BackQueues& backQueues) override;
  std::vector<types::URL> extractBatch(BackQueues& backQueues,
                                       std::size_t maxCount) override;
};

}  // namespace components

}  // namespace crawler