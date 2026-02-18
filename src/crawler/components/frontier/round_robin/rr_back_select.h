#pragma once

#include "../back_queues.h"
#include "../i_back_selector.h"

namespace crawler {

namespace components {

class RoundRobinBackSelector : public IBackSelector {
 public:
  RoundRobinBackSelector() = default;

  std::optional<types::URL> extract(BackQueues& backQueues) override;
  std::vector<types::URL> extractBatch(BackQueues& backQueues,
                                       std::size_t maxCount) override;

 private:
  std::size_t pointer_{0};

  std::mutex mutex_;
};

}  // namespace components

}  // namespace crawler