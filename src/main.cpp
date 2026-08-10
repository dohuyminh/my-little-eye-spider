#include <print>
#include "crawler/types/url.h"
#include "crawler/setup/frontier/politeness_tracker.h"
#include "crawler/database/postgresql_adapter.h"
#include "dotenv.h"

using namespace crawler::types;
using namespace crawler::setup;
using namespace crawler::database;

std::string getTestConnectionString(std::string_view databaseName) {

  dotenv::env.load_dotenv("../.env");

  const auto host = std::string{dotenv::env["POSTGRESQL_DB_HOST"]};
  const auto port = std::string{dotenv::env["POSTGRESQL_DB_PORT"]};
  const auto user = std::string{dotenv::env["POSTGRESQL_DB_USERNAME"]};
  const auto password = std::string{dotenv::env["POSTGRESQL_DB_PASSWORD"]};

  if (host.empty() || port.empty() || user.empty() || password.empty()) {
    throw std::invalid_argument(
        "[getTestConnectionString] PostgreSQL test configuration is incomplete");
  }

  return std::format("host={} port={} dbname={} user={} password={}", host,
                     port, databaseName, user, password);
}

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
  
  PostgreSQLConnectionPool pool{  
      getTestConnectionString("postgres"),
      1
  };

  std::atomic acquired{false};

  std::thread waiter([&pool, &acquired]() {
      auto lease = pool.acquireConnection();

      acquired.store(true, std::memory_order_release);

      // `lease` remains alive until the thread finishes.
  });

  {
      // Acquire the only connection in the pool.
      auto first = pool.acquireConnection();

      // Give the waiter an opportunity to block on acquireConnection().
      std::this_thread::sleep_for(std::chrono::milliseconds(250));

      // `first` goes out of scope here.
      // Its destructor automatically returns the connection to the pool.
  }

  // The waiter should now be able to acquire the returned connection.
  waiter.join();

  assert(acquired.load(std::memory_order_acquire));

  return 0;
} 
