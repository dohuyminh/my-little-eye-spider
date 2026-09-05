#include "politeness_tracker.h"

#include "services/http_get_handler.h"
#include "services/url.h"

using RobotsTxtRepr = crawler::services::url::RobotsTxtRepr;

namespace crawler::setup {

PolitenessTracker& PolitenessTracker::get() {
  static PolitenessTracker instance;
  return instance;
}

bool PolitenessTracker::domainIsLocked(const std::string& domain) {
  {
    std::shared_lock<std::shared_mutex> searchLock(lockedDomainsMutex_);
    return lockedDomain_.count(domain);
  }
}

const services::url::RobotsTxtRepr& PolitenessTracker::getRobotsTxt(
    const types::URL& url) {
  std::string fullHost{url.host()};

  // Check if we already have robots.txt for this domain
  auto it{robotsTxtLookup_.find(fullHost)};
  if (it != robotsTxtLookup_.end()) {
    return it->second;
  }

  // Construct full robots.txt URL using scheme + full hostname + port +
  // /robots.txt
  std::string robotsUrl{url.scheme()};
  if (!robotsUrl.empty()) robotsUrl += "://";
  robotsUrl += fullHost;
  if (!url.port().empty()) robotsUrl += ":" + url.port();
  robotsUrl += "/robots.txt";

  // Fetch robots.txt
  services::html::HTTPGETHandler downloader;
  types::URL robotsTextUrl{robotsUrl};
  services::curl::Response response{downloader(robotsTextUrl)};

  if (response.code() != 200) {
    robotsTxtLookup_[fullHost] = {
        {"allow", nlohmann::json::array()},
        {"disallow", nlohmann::json::array()}};  // empty JSON object
    return robotsTxtLookup_[fullHost];
  }

  services::url::ParseResult<services::url::RobotsTxtRepr> result{
      services::url::parseRobotsTxt(response.content())};
  if (!result.has_value()) {
    robotsTxtLookup_[fullHost] = {
        {"allow", nlohmann::json::array()},
        {"disallow", nlohmann::json::array()}};  // empty JSON object
    return robotsTxtLookup_[fullHost];
  }

  robotsTxtLookup_[fullHost] = result.value();
  return robotsTxtLookup_[fullHost];
}

void PolitenessTracker::setRobotsTxtProvider(RobotsTxtProvider provider) {
  std::unique_lock<std::shared_mutex> lock(lockedDomainsMutex_);
  robotsTxtProvider_ = std::move(provider);
}

void PolitenessTracker::resetRobotsTxtProvider() {
  std::unique_lock<std::shared_mutex> lock(lockedDomainsMutex_);
  robotsTxtProvider_ = nullptr;
}

void PolitenessTracker::clearLockedDomains() {
  std::unique_lock<std::shared_mutex> lock(lockedDomainsMutex_);
  lockedDomain_.clear();
}

void PolitenessTracker::clearRobotsTxtCache() {
  std::unique_lock<std::shared_mutex> lock(lockedDomainsMutex_);
  robotsTxtLookup_.clear();
}

DomainStatus PolitenessTracker::handleURL(const types::URL& url) {
  // get the robots.txt content for the URL's domain
  services::url::RobotsTxtRepr robotsTxt;

  if (robotsTxtProvider_) {
    robotsTxt = robotsTxtProvider_(url);
  } else {
    robotsTxt = getRobotsTxt(url);
  }

  std::string urlStr{url.to_string()};
  std::string fullHost{url.host()};

  // if the URL is in Disallow list
  for (const auto& disallowPattern : robotsTxt["disallow"]) {
    if (services::url::urlIsDisallowed(urlStr, disallowPattern)) {
      return DomainStatus::DISALLOWED;
    }
  }

  // check if the domain is already locked
  {
    std::shared_lock<std::shared_mutex> searchLock(lockedDomainsMutex_);
    if (lockedDomain_.count(fullHost)) {
      return DomainStatus::LOCKED;
    }
  }

  // if the domain is currently available but needs to go through
  // cooldown to ensure politeness, run the delay
  if (robotsTxt.contains("crawl-delay")) {
    try {
      auto seconds{static_cast<int>(robotsTxt["crawl-delay"].get<double>())};

      // lock domain
      {
        std::unique_lock<std::shared_mutex> insertLock(lockedDomainsMutex_);
        if (lockedDomain_.count(fullHost)) {
          return DomainStatus::LOCKED;
        }
        lockedDomain_.insert(fullHost);
      }

      // set a delay for release
      delayPool_.enqueue<false>(
          [this, domain = fullHost,
           seconds = seconds]() mutable -> boost::asio::awaitable<void> {
            // wait for specified amount of time
            co_await boost::asio::steady_timer(
                co_await boost::asio::this_coro::executor,
                std::chrono::seconds(seconds))
                .async_wait(boost::asio::use_awaitable);

            // remove domain from locked set
            std::unique_lock<std::shared_mutex> freeLock(
                this->lockedDomainsMutex_);
            this->lockedDomain_.erase(domain);

            co_return;
          });
    } catch (const std::exception&) {
      // If parsing crawl-delay fails, treat as no delay
    }
  }

  // otherwise, it's allowed and thus can be crawled
  return DomainStatus::ALLOWED;
}

}  // namespace crawler::setup
