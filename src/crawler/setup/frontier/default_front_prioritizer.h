#pragma once

#include <atomic>
#include <shared_mutex>
#include <unordered_set>

#include "types/url.h"

namespace crawler {

namespace setup {

class DefaultFrontPrioritizer {
 public:
  DefaultFrontPrioritizer(double significantPercent = 0.7);

  std::pair<types::URL, std::size_t> selectQueue(const std::string& url);

 private:
  enum class QueueCategory : std::size_t {
    SHALLOW_URLS = 0,
    DEEP_URLS = 1,
    PARAMETER_HEAVY_URLS = 2,
    UNEXPLORED_DOMAINS = 3
  };

  double significantPercent_{0.7};

  std::unordered_set<std::string> seenDomains_;

  std::atomic<std::size_t> longestURLDepth_{0};
  std::atomic<std::size_t> mostQueryParams_{0};

  std::shared_mutex domainLookupMutex_;
};

}  // namespace setup

}  // namespace crawler
