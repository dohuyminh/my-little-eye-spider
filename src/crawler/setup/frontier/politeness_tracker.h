#pragma once

#include <functional>
#include <nlohmann/json.hpp>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "services/coroutine_thread_pool.h"
#include "types/url.h"

namespace crawler::setup {

enum class DomainStatus { LOCKED, ALLOWED, DISALLOWED };

class PolitenessTracker
    : public std::enable_shared_from_this<PolitenessTracker> {
 public:
  using RobotsTxtProvider = std::function<services::url::RobotsTxtRepr(
      const types::URL& url)>;

  static PolitenessTracker& get();

  bool domainIsLocked(const std::string& domain);

  DomainStatus handleURL(const types::URL& url);

  void setRobotsTxtProvider(RobotsTxtProvider provider);
  void resetRobotsTxtProvider();
  void clearLockedDomains();
  void clearRobotsTxtCache();

 private:
  std::unordered_set<std::string> lockedDomain_{};
  std::shared_mutex lockedDomainsMutex_{};
  std::unordered_map<std::string, services::url::RobotsTxtRepr>
      robotsTxtLookup_{};
  RobotsTxtProvider robotsTxtProvider_{};
  services::concurrency::CoroutineThreadPool delayPool_{};

  PolitenessTracker() = default;

  const services::url::RobotsTxtRepr& getRobotsTxt(const types::URL& url);

 public:
  PolitenessTracker(const PolitenessTracker&) = delete;
  void operator=(const PolitenessTracker&) = delete;
};

}  // namespace crawler::setup
