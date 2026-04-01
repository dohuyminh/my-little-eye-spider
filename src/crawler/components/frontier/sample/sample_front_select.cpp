#include "sample_front_select.h"

namespace crawler {

namespace components {

std::optional<types::URL> SampleFrontSelector::extract(
    IFrontSelector::FrontQueueContainer& frontQueues) {
  return frontQueues.dequeue(0);
}

std::vector<types::URL> SampleFrontSelector::extractBatch(
    IFrontSelector::FrontQueueContainer& frontQueues, std::size_t maxCount) {
  std::vector<types::URL> result;
  result.reserve(maxCount);
  for (std::size_t i = 0; i < maxCount; ++i) {
    auto urlOpt = frontQueues.dequeue(0);
    if (!urlOpt.has_value()) break;
    result.push_back(std::move(urlOpt.value()));
  }
  return result;
}

}  // namespace components

}  // namespace crawler