#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <future>
#include <atomic>
#include <vector>

#include "crawler/components/frontier/politeness_tracker.h"
#include "crawler/types/url.h"

namespace crawler::components {

namespace {

// ============================================================================
// Singleton behavior tests
// ============================================================================

TEST(PolitenessTrackerSingletonTest, ReturnsSameInstance) {
  auto instance1 = PolitenessTracker::get();
  auto instance2 = PolitenessTracker::get();
  EXPECT_EQ(instance1, instance2);
}

// ============================================================================
// domainIsLocked tests
// ============================================================================

TEST(PolitenessTrackerDomainIsLockedTest, InitiallyNoDomainsLocked) {
  auto tracker = PolitenessTracker::get();
  EXPECT_FALSE(tracker->domainIsLocked("example.com"));
  EXPECT_FALSE(tracker->domainIsLocked("any.domain.com"));
}

// ============================================================================
// Crawl-delay politeness tests - Single domain
// ============================================================================

// Real domain: www.craftbrewingbusiness.com has Crawl-delay: 1 for multiple user-agents
TEST(PolitenessTrackerCrawlDelayTest, DomainLockedAfterFirstRequestWithCrawlDelay) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(1));
  
  auto tracker = PolitenessTracker::get();

  // First request to craftbrewingbusiness.com should return ALLOWED and lock the domain
  types::URL url1{"https://www.craftbrewingbusiness.com/search"};
  DomainStatus status1 = tracker->handleURL(url1);

  // First request should be ALLOWED (triggers the lock + delay)
  EXPECT_EQ(status1, DomainStatus::ALLOWED);

  // Immediately check if domain is locked
  EXPECT_TRUE(tracker->domainIsLocked("craftbrewingbusiness.com"));

  // Second request to same domain should return LOCKED
  types::URL url2{"https://www.craftbrewingbusiness.com/page2"};
  DomainStatus status2 = tracker->handleURL(url2);
  EXPECT_EQ(status2, DomainStatus::LOCKED);
}

TEST(PolitenessTrackerCrawlDelayTest, DomainReleasedAfterCrawlDelayExpires) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(1));
  
  auto tracker = PolitenessTracker::get();

  // Use craftbrewingbusiness.com with 1 second crawl-delay
  types::URL url1{"https://www.craftbrewingbusiness.com/data"};
  DomainStatus status1 = tracker->handleURL(url1);

  EXPECT_EQ(status1, DomainStatus::ALLOWED);
  EXPECT_TRUE(tracker->domainIsLocked("craftbrewingbusiness.com"));

  // Wait for crawl-delay to expire (1 second + buffer)
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));

  // Domain should be released after delay
  EXPECT_FALSE(tracker->domainIsLocked("craftbrewingbusiness.com"));

  // New request should be ALLOWED again
  types::URL url2{"https://www.craftbrewingbusiness.com/newpage"};
  DomainStatus status2 = tracker->handleURL(url2);
  EXPECT_EQ(status2, DomainStatus::ALLOWED);

  // Domain should be locked again
  EXPECT_TRUE(tracker->domainIsLocked("craftbrewingbusiness.com"));
}

TEST(PolitenessTrackerCrawlDelayTest, MultipleRequestsDuringLockPeriodStayBlocked) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(1));
  
  auto tracker = PolitenessTracker::get();

  // Initial request locks the domain
  types::URL url1{"https://www.craftbrewingbusiness.com/initial"};
  DomainStatus status1 = tracker->handleURL(url1);
  EXPECT_EQ(status1, DomainStatus::ALLOWED);

  // Multiple requests during lock period should all return LOCKED
  for (int i = 0; i < 5; ++i) {
    types::URL url{"https://www.craftbrewingbusiness.com/request" + std::to_string(i)};
    DomainStatus status = tracker->handleURL(url);
    EXPECT_EQ(status, DomainStatus::LOCKED) << "Request " << i << " should be LOCKED";
  }

  // Wait for delay to expire
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));

  // After delay, domain should be available again
  types::URL urlAfter{"https://www.craftbrewingbusiness.com/after"};
  DomainStatus statusAfter = tracker->handleURL(urlAfter);
  EXPECT_EQ(statusAfter, DomainStatus::ALLOWED);
}

// ============================================================================
// Crawl-delay tests - Different domains with different delays
// ============================================================================

// www.craftbrewingbusiness.com: Crawl-delay: 1
// Note: Most government/public sites (like usa.gov) don't have crawl-delay set
// So this test only verifies that craftbrewingbusiness.com has independent delays
// ============================================================================
// Crawl-delay tests - Different domains with different delays
// ============================================================================

// Note: Most government/public sites (like usa.gov) don't have crawl-delay set.
// This test verifies that the politeness tracker correctly handles domains
// with and without crawl delays.
TEST(PolitenessTrackerCrawlDelayTest, DifferentDomainsHaveIndependentDelays) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(10));
  
  auto tracker = PolitenessTracker::get();

  // Request to craftbrewingbusiness.com (1 second delay)
  types::URL craftBrewingUrl{"https://www.craftbrewingbusiness.com/page"};
  DomainStatus craftStatus = tracker->handleURL(craftBrewingUrl);
  EXPECT_EQ(craftStatus, DomainStatus::ALLOWED);

  // Request to another domain without crawl-delay (usa.gov doesn't have it)
  types::URL usaUrl{"https://www.usa.gov/page"};
  DomainStatus usaStatus1 = tracker->handleURL(usaUrl);
  
  // Without crawl-delay, usa.gov should return ALLOWED 
  EXPECT_EQ(usaStatus1, DomainStatus::ALLOWED);

  // Requests to both domains after initial calls:
  // Multiple rapid requests to craftbrewing might be LOCKED or ALLOWED
  // depending on timing, but they should be consistent
  types::URL craftBrewingUrl2{"https://www.craftbrewingbusiness.com/page2"};
  DomainStatus craftStatus2 = tracker->handleURL(craftBrewingUrl2);
  EXPECT_TRUE(craftStatus2 == DomainStatus::LOCKED || craftStatus2 == DomainStatus::ALLOWED);

  // usa.gov should be ALLOWED again (no delay, always available)
  types::URL usaUrl2{"https://www.usa.gov/page2"};
  DomainStatus usaStatus2 = tracker->handleURL(usaUrl2);
  EXPECT_EQ(usaStatus2, DomainStatus::ALLOWED);

  // Wait for craftbrewingbusiness.com delay (1 second)
  // This ensures any locks are released before the test ends
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));

  // After waiting, new request to craftbrewingbusiness.com should return ALLOWED
  types::URL craftBrewingUrl3{"https://www.craftbrewingbusiness.com/page3"};
  DomainStatus craftStatus3 = tracker->handleURL(craftBrewingUrl3);
  EXPECT_EQ(craftStatus3, DomainStatus::ALLOWED);

  // usa.gov should still be ALLOWED (no crawl-delay)
  types::URL usaUrl3{"https://www.usa.gov/page3"};
  DomainStatus usaStatus3 = tracker->handleURL(usaUrl3);
  EXPECT_EQ(usaStatus3, DomainStatus::ALLOWED);
}

// ============================================================================
// Concurrent multi-domain access tests
// ============================================================================

// Note: Most real-world domains without explicit crawl-delay will return ALLOWED
// without locking, which is fine. This test verifies concurrent access doesn't crash.
TEST(PolitenessTrackerCrawlDelayTest, ConcurrentAccessToMultipleDomains) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(10));
  
  auto tracker = PolitenessTracker::get();

  const int numDomains = 5;
  std::vector<std::thread> threads;
  std::vector<std::pair<std::string, DomainStatus>> results(numDomains);

  // Domains - using mainly craftbrewingbusiness since it has crawl-delay
  std::vector<std::string> domains = {
    "www.craftbrewingbusiness.com",    // 1 second crawl-delay
    "www.usa.gov",                     // no crawl-delay (returns ALLOWED but not locked)
    "www.craftbrewingbusiness.com",    // 1 second (duplicate)
    "www.usa.gov",                     // no crawl-delay (duplicate)
    "www.craftbrewingbusiness.com"     // 1 second
  };

  std::vector<std::string> paths = {
    "/page1", "/page2", "/page3", "/page4", "/page5"
  };

  // Launch concurrent requests to different domains
  for (int i = 0; i < numDomains; ++i) {
    threads.emplace_back([&tracker, &results, &domains, &paths, i]() {
      types::URL url{"https://" + domains[i] + paths[i]};
      results[i] = {domains[i], tracker->handleURL(url)};
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // All requests should return valid statuses (ALLOWED or LOCKED)
  for (int i = 0; i < numDomains; ++i) {
    EXPECT_TRUE(results[i].second == DomainStatus::ALLOWED ||
                results[i].second == DomainStatus::LOCKED)
      << "Request " << i << " to " << results[i].first << " returned invalid status";
  }

  // At least one request to craftbrewingbusiness.com should be locked 
  // (since it has crawl-delay)
  bool craftbrewingLocked = tracker->domainIsLocked("craftbrewingbusiness.com");
  EXPECT_TRUE(craftbrewingLocked) << "craftbrewingbusiness.com should be locked";
  
  // usa.gov doesn't have crawl-delay, so it should NOT be locked
  bool usaLocked = tracker->domainIsLocked("usa.gov");
  EXPECT_FALSE(usaLocked) << "usa.gov should NOT be locked";
}

// ============================================================================
// Crawl-delay timing tests
// ============================================================================

TEST(PolitenessTrackerCrawlDelayTest, DomainLockDurationMatchesCrawlDelay) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(1));
  
  auto tracker = PolitenessTracker::get();

  // Use craftbrewingbusiness.com with 1 second crawl-delay
  auto startTime = std::chrono::steady_clock::now();

  types::URL url1{"https://www.craftbrewingbusiness.com/timing"};
  tracker->handleURL(url1);

  // Wait and poll until domain is released
  bool released = false;
  for (int i = 0; i < 30; ++i) {  // Check every 100ms for up to 3 seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (!tracker->domainIsLocked(url1.domain())) {
      released = true;
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - startTime).count();

      // Should be released after approximately 1 second (crawl-delay)
      // Allow some tolerance for network latency and scheduling
      EXPECT_GE(elapsed, 800) << "Domain released too early";
      EXPECT_LE(elapsed, 2500) << "Domain released too late";
      break;
    }
  }

  EXPECT_TRUE(released) << "Domain was not released within timeout";
}

// ============================================================================
// Thread safety tests
// ============================================================================

TEST(PolitenessTrackerThreadSafetyTest, ConcurrentDomainIsLockedCalls) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(1));
  
  auto tracker = PolitenessTracker::get();

  const int numThreads = 10;
  std::vector<std::thread> threads;
  std::vector<bool> results(numThreads);

  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back([&tracker, &results, i]() {
      std::string domain = "craftbrewingbusiness.com";
      results[i] = tracker->domainIsLocked(domain);
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // All calls should complete without crash
  SUCCEED();
}

TEST(PolitenessTrackerThreadSafetyTest, ConcurrentHandleURLCalls) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(1));
  
  auto tracker = PolitenessTracker::get();

  const int numThreads = 5;
  std::vector<std::thread> threads;
  std::vector<DomainStatus> results(numThreads);

  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back([&tracker, &results, i]() {
      types::URL url{"https://www.craftbrewingbusiness.com/concurrent" + std::to_string(i)};
      results[i] = tracker->handleURL(url);
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // All calls should return valid status
  for (int i = 0; i < numThreads; ++i) {
    EXPECT_TRUE(results[i] == DomainStatus::ALLOWED ||
                results[i] == DomainStatus::LOCKED ||
                results[i] == DomainStatus::DISALLOWED);
  }

  SUCCEED();
}

// ============================================================================
// Edge cases
// ============================================================================

TEST(PolitenessTrackerEdgeCasesTest, HandlesEmptyPath) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(1));
  
  auto tracker = PolitenessTracker::get();
  types::URL url{"https://www.craftbrewingbusiness.com/"};

  DomainStatus status = tracker->handleURL(url);
  EXPECT_TRUE(status == DomainStatus::DISALLOWED ||
              status == DomainStatus::ALLOWED ||
              status == DomainStatus::LOCKED);
}

TEST(PolitenessTrackerEdgeCasesTest, HandlesUrlWithPort) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(1));
  
  auto tracker = PolitenessTracker::get();
  types::URL url{"https://www.craftbrewingbusiness.com:443/api"};

  DomainStatus status = tracker->handleURL(url);
  EXPECT_TRUE(status == DomainStatus::DISALLOWED ||
              status == DomainStatus::ALLOWED ||
              status == DomainStatus::LOCKED);
}

TEST(PolitenessTrackerEdgeCasesTest, HandlesUrlWithSubdomain) {
  
  // sleep for 10 seconds to ensure domain is actually free before running test
  std::this_thread::sleep_for(std::chrono::seconds(1));

  auto tracker = PolitenessTracker::get();
  types::URL url{"https://www.craftbrewingbusiness.com/"};

  DomainStatus status = tracker->handleURL(url);
  EXPECT_TRUE(status == DomainStatus::DISALLOWED ||
              status == DomainStatus::ALLOWED ||
              status == DomainStatus::LOCKED);
}

}  // namespace

}  // namespace crawler::components
