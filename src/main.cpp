#include <print>
#include "crawler/types/url.h"
#include "crawler/setup/frontier/politeness_tracker.h"

using namespace crawler::types;
using namespace crawler::setup;

int main() {
  
    // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(1));

  auto& tracker = PolitenessTracker::get();

  // First request to craftbrewingbusiness.com should return ALLOWED and lock the domain
  URL url1{"https://www.craftbrewingbusiness.com/search"};
  DomainStatus status1 = tracker.handleURL(url1);

  // First request should be ALLOWED (triggers the lock + delay)

  // Immediately check if domain is locked (using full hostname)

  // Second request to same domain should return LOCKED
  URL url2{"https://www.craftbrewingbusiness.com/page2"};
  DomainStatus status2 = tracker.handleURL(url2);

  std::println("{} {}", status1 == DomainStatus::ALLOWED, status2 == DomainStatus::LOCKED);
 
  std::println("long: {}", sizeof(long));

  return 0;
} 
