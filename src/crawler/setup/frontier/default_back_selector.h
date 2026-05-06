#pragma once

#include "components/frontier/multiqueue_containers.h"
#include "types/url.h"

namespace crawler {

namespace setup {

class DefaultBackSelector {
 public:
  using BackQueueContainer =
      components::MultiQueueContainers<components::StdQueueWrapper<types::URL>>;

  DefaultBackSelector() = default;

  std::optional<types::URL> extract(BackQueueContainer& backQueues);
  std::vector<types::URL> extractBatch(BackQueueContainer& backQueues,
                                       std::size_t maxCount);

 private:
  std::size_t ptr_{0};
};

}  // namespace setup

}  // namespace crawler
