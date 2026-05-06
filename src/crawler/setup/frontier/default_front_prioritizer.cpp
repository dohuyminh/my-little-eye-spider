#include "default_front_prioritizer.h"

#include <cstddef>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>

namespace crawler::setup {

DefaultFrontPrioritizer::DefaultFrontPrioritizer(double significantPercent)
    : significantPercent_(significantPercent) {
  if (significantPercent_ > 1.0 || significantPercent_ < 0.0) {
    throw std::invalid_argument(
        "Default Front Prioritizer: significant percent must be a value "
        "between 0 and 1");
  }
}

std::pair<types::URL, std::size_t> DefaultFrontPrioritizer::selectQueue(
    const std::string& url) {
  types::URL urlObj{url};

  {
    std::shared_lock<std::shared_mutex> lookupLock{domainLookupMutex_};
    if (!seenDomains_.count(urlObj.domain())) {
      return {urlObj,
              static_cast<std::size_t>(QueueCategory::UNEXPLORED_DOMAINS)};
    }
  }

  {
    std::unique_lock<std::shared_mutex> insertLock{domainLookupMutex_};
    if (!seenDomains_.count(urlObj.domain())) {
      return {urlObj,
              static_cast<std::size_t>(QueueCategory::UNEXPLORED_DOMAINS)};
    }
    seenDomains_.insert(urlObj.domain());
  }

  std::pair<types::URL, std::size_t> result{
      urlObj, static_cast<std::size_t>(QueueCategory::SHALLOW_URLS)};

  std::size_t currentGreatestDepth{longestURLDepth_.load()};
  std::size_t currentGreatestNumQueries{mostQueryParams_.load()};

  auto urlDepthThreshold =
      static_cast<std::size_t>(currentGreatestDepth * significantPercent_);
  auto urlNumQueriesThreshold =
      static_cast<std::size_t>(currentGreatestNumQueries * significantPercent_);

  std::size_t urlNumQueries{urlObj.queries().size()};
  std::size_t urlDepth{urlObj.path().size()};

  if (urlDepth > urlDepthThreshold) {
    result.second = static_cast<std::size_t>(QueueCategory::DEEP_URLS);
  }

  else if (urlNumQueries > urlNumQueriesThreshold) {
    result.second =
        static_cast<std::size_t>(QueueCategory::PARAMETER_HEAVY_URLS);
  }

  std::size_t prevGreatestNumQueries{currentGreatestNumQueries};
  std::size_t prevGreatestDepth{currentGreatestDepth};

  while (prevGreatestDepth < urlDepth &&
         !longestURLDepth_.compare_exchange_weak(prevGreatestDepth, urlDepth));
  while (prevGreatestNumQueries < urlNumQueries &&
         !mostQueryParams_.compare_exchange_weak(prevGreatestNumQueries,
                                                 urlNumQueries));

  return result;
}

}  // namespace crawler::setup
