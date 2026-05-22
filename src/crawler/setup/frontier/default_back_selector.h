#pragma once

#include "components/frontier/multiqueue_containers.h"
#include "setup/frontier/default_frontier_return_type.h"

namespace crawler {

namespace setup {

class DefaultBackSelector {
 public:
  using BackQueueContainer = components::MultiQueueContainers<
      components::StdQueueWrapper<DefaultFrontierReturnType>>;

  DefaultBackSelector() = default;

  std::optional<DefaultFrontierReturnType> extract(
      BackQueueContainer& backQueues);
  std::vector<DefaultFrontierReturnType> extractBatch(
      BackQueueContainer& backQueues, std::size_t maxCount);

 private:
  std::size_t ptr_{0};
};

}  // namespace setup

}  // namespace crawler
