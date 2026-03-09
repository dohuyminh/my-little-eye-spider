#include "rr_front_select.h"

namespace crawler {

namespace components {

std::optional<types::URL> RoundRobinFrontSelector::extract(
    FrontQueues& frontQueues) {
  std::unique_lock<std::mutex> lock(mutex_);

  const std::size_t nQueues = frontQueues.numQueues();

  for (std::size_t i = 0; i < nQueues; ++i) {
    const std::size_t currentIndex = pointer_;
    pointer_ = (pointer_ + 1) % nQueues;

    auto urlOpt = frontQueues.selectAndPop(currentIndex);
    if (urlOpt.has_value()) {
      return urlOpt;
    }
  }

  return std::nullopt;
}

std::vector<types::URL> RoundRobinFrontSelector::extractBatch(
    FrontQueues& frontQueues, std::size_t maxCount) {
  std::unique_lock<std::mutex> lock(mutex_);

  std::vector<types::URL> result;
  result.reserve(maxCount);

  const std::size_t nQueues = frontQueues.numQueues();
  for (std::size_t collected = 0; collected < maxCount;) {
    bool gotAny = false;
    for (std::size_t i = 0; i < nQueues && collected < maxCount; ++i) {
      const std::size_t currentIndex = pointer_;
      pointer_ = (pointer_ + 1) % nQueues;

      auto urlOpt = frontQueues.selectAndPop(currentIndex);
      if (urlOpt.has_value()) {
        result.emplace_back(std::move(*urlOpt));
        ++collected;
        gotAny = true;
      }
    }
    if (!gotAny) break;
  }

  return result;
}

}  // namespace components

}  // namespace crawler