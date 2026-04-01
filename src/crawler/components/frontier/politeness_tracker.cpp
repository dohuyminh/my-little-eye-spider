#include "politeness_tracker.h"

#include "re2/re2.h"

#include "services/url.h"
#include "services/html_downloader.h"

using RobotsTxtRepr = crawler::services::url::RobotsTxtRepr;

namespace crawler {

namespace components {

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

const services::url::RobotsTxtRepr& PolitenessTracker::getRobotsTxt(const types::URL& url) {
  const std::string& domain{url.domain()};

  // Check if we already have robots.txt for this domain
  auto it{robotsTxtLookup_.find(domain)};
  if (it != robotsTxtLookup_.end()) {
    return it->second;
  }

  // Fetch robots.txt
  services::html::Downloader downloader;
  types::URL domainRoot{url};
  static std::string robotsTxtPath{"/robots.txt"};
  domainRoot.applyPath(robotsTxtPath);

  services::curl::Response response{downloader(domainRoot)};

  if (response.code() != 200) {
    robotsTxtLookup_[domain] = RobotsTxtRepr{}; // empty JSON object
    return robotsTxtLookup_[domain];
  }

  services::url::ParseResult<services::url::RobotsTxtRepr> result{services::url::parseRobotsTxt(response.content())};
  if (!result.has_value()) {
    robotsTxtLookup_[domain] = RobotsTxtRepr{}; // empty JSON object
    return robotsTxtLookup_[domain];
  }

  robotsTxtLookup_[domain] = result.value();
  return robotsTxtLookup_[domain];
}

DomainStatus PolitenessTracker::handleURL(const types::URL& url) {
  
  // get the robots.txt content for the URL's domain
  const auto& robotsTxt{getRobotsTxt(url)};

  std::string urlStr{url.to_string()};

  // if the URL is in Disallow list
  for (const std::string& disallowPattern: robotsTxt["disallow"]) {
    if (services::url::urlIsDisallowed(urlStr, disallowPattern)) {
      return DomainStatus::DISALLOWED;
    }
  }
  
  // check if the domain is already locked
  {
    std::shared_lock<std::shared_mutex> searchLock(lockedDomainsMutex_);
    if (lockedDomain_.count(url.domain())) {
      return DomainStatus::LOCKED;
    }
  }

  // if the domain is currently available but needs to go through 
  // cooldown to ensure politeness, run the delay
  if (robotsTxt.contains("crawl-delay")) {
    auto seconds{robotsTxt["crawl-delay"].get<int>()};
    
    // lock domain
    {
      std::unique_lock<std::shared_mutex> insertLock(lockedDomainsMutex_);
      if (lockedDomain_.count(url.domain())) {
        return DomainStatus::LOCKED;
      }
      lockedDomain_.insert(url.domain());
    }
    
    // set a delay for release
    delayPool_.enqueue([this, domain = url.domain(), seconds]() mutable -> boost::asio::awaitable<void> {
      // wait for specified amount of time
      co_await boost::asio::steady_timer(
        co_await boost::asio::this_coro::executor, std::chrono::seconds(seconds)
      ).async_wait(boost::asio::use_awaitable);

      // remove domain from locked set
      std::unique_lock<std::shared_mutex> insertLock(lockedDomainsMutex_);
      lockedDomain_.erase(domain);
      
      co_return;
    });
  } 

  // otherwise, it's allowed and thus can be crawled
  return DomainStatus::ALLOWED;
}

}

}