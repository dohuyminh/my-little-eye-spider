#include "sample_back_select.h"

namespace crawler {

namespace components {

std::optional<types::URL> SampleBackSelector::extract(
    IBackSelector::BackQueueContainer& backQueues) {
  return backQueues.dequeue(0);
}

std::vector<types::URL> SampleBackSelector::extractBatch(
    IBackSelector::BackQueueContainer& backQueues, std::size_t maxCount) {
  std::vector<types::URL> result;
  result.reserve(maxCount);
  for (std::size_t i = 0; i < maxCount; ++i) {
    auto urlOpt = backQueues.dequeue(0);
    if (!urlOpt.has_value()) break;
    result.push_back(std::move(urlOpt.value()));
  }
  return result;
}

}  // namespace components

}  // namespace crawler