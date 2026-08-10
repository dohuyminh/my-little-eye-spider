#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

#include "crawler/database/postgresql_adapter.h"
#include "dotenv.h"

using namespace crawler::database;

namespace postgres_test {

namespace {

class ScopedEnvVar {
 public:
  ScopedEnvVar(const char* key, std::string value)
      : key_(key), previous_value_(std::getenv(key)) {
    if (value.empty()) {
      unsetenv(key_.c_str());
    } else {
      setenv(key_.c_str(), value.c_str(), 1);
    }
  }

  ~ScopedEnvVar() {
    if (previous_value_) {
      setenv(key_.c_str(), previous_value_->c_str(), 1);
    } else {
      unsetenv(key_.c_str());
    }
  }

 private:
  std::string key_;
  std::optional<std::string> previous_value_;
};

std::string loadDotEnvFile() {
  const std::array<std::string, 3> candidates{
      "../../.env",
      "../.env",
      ".env"
  };

  for (const auto& candidate : candidates) {
    if (std::filesystem::exists(candidate)) {
      dotenv::env.load_dotenv(candidate);
      return candidate;
    }
  }

  dotenv::env.load_dotenv();
  return {};
}

std::string getTestConnectionString(std::string_view databaseName) {
  loadDotEnvFile();

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

bool canReachPostgres() {
  try {
    pqxx::connection conn{getTestConnectionString("postgres")};
    return conn.is_open();
  } catch (...) {
    return false;
  }
}

}  // namespace

TEST(PostgreSQLConnectionPoolTest, ConstructorRejectsZeroConnections) {
  EXPECT_THROW((PostgreSQLConnectionPool{"host=localhost port=5432", 0}),
               std::invalid_argument);
}

TEST(PostgreSQLConnectionPoolTest, ConstructorRejectsInvalidConnectionString) {
  EXPECT_THROW((PostgreSQLConnectionPool{"host=invalid.invalid port=1", 1}),
               std::runtime_error);
}

TEST(PostgreSQLConnectionPoolTest, ConstructorCreatesValidPoolWhenDatabaseIsAvailable) {
  if (!canReachPostgres()) {
    GTEST_SKIP() << "PostgreSQL test database is not reachable";
  }

  EXPECT_NO_THROW(
      PostgreSQLConnectionPool(getTestConnectionString("postgres"), 2));
}

TEST(PostgreSQLConnectionPoolTest, LeaseExposesAUsableConnection) {
  if (!canReachPostgres()) {
    GTEST_SKIP() << "PostgreSQL test database is not reachable";
  }

  PostgreSQLConnectionPool pool{getTestConnectionString("postgres"), 1};
  auto lease = pool.acquireConnection();

  ASSERT_NE(lease.operator->(), nullptr);
  EXPECT_TRUE((*lease).is_open());
}

TEST(PostgreSQLConnectionPoolTest, MultipleLeasesFromSamePoolUseDistinctConnections) {
  if (!canReachPostgres()) {
    GTEST_SKIP() << "PostgreSQL test database is not reachable";
  }

  PostgreSQLConnectionPool pool{getTestConnectionString("postgres"), 2};
  auto first = pool.acquireConnection();
  auto second = pool.acquireConnection();

  ASSERT_NE(first.operator->(), nullptr);
  ASSERT_NE(second.operator->(), nullptr);
  EXPECT_NE(first.operator->(), second.operator->());
}

TEST(PostgreSQLConnectionPoolTest, AcquireWaitsUntilAConnectionIsReleased) {
  if (!canReachPostgres()) {
    GTEST_SKIP() << "PostgreSQL test database is not reachable";
  }

  PostgreSQLConnectionPool pool{getTestConnectionString("postgres"), 1};
  std::atomic<bool> acquired{false};

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

  waiter.join();
  EXPECT_TRUE(acquired.load(std::memory_order_acquire));
}

TEST(PostgreSQLAdapterTest, ExecuteSelectAndInsertWorkWithARealDatabase) {
  if (!canReachPostgres()) {
    GTEST_SKIP() << "PostgreSQL test database is not reachable";
  }

  PostgreSQLAdapter adapter{getTestConnectionString("postgres")};

  auto drop = adapter.execute("DROP TABLE IF EXISTS adapter_test_table", true);
  ASSERT_TRUE(drop.has_value()) << "dropping test table should succeed";

  auto create = adapter.execute(
      "CREATE TABLE adapter_test_table (id INT PRIMARY KEY, name VARCHAR(50))",
      true);
  ASSERT_TRUE(create.has_value()) << "creating test table should succeed";

  auto insert = adapter.execute(
      "INSERT INTO adapter_test_table (id, name) VALUES (1, 'hello')", true);
  ASSERT_TRUE(insert.has_value()) << "insert should succeed";

  auto select = adapter.execute("SELECT * FROM adapter_test_table", false);
  ASSERT_TRUE(select.has_value()) << "select should succeed";
  EXPECT_EQ(select.value().size(), 1);

  auto cleanup = adapter.execute("DROP TABLE adapter_test_table", true);
  ASSERT_TRUE(cleanup.has_value()) << "cleanup should succeed";
}

TEST(PostgreSQLAdapterTest, InvalidSqlReturnsUnexpected) {
  if (!canReachPostgres()) {
    GTEST_SKIP() << "PostgreSQL test database is not reachable";
  }

  PostgreSQLAdapter adapter{getTestConnectionString("postgres")};

  auto result = adapter.execute("SELECT * FROM definitely_missing_table", false);

  EXPECT_FALSE(result.has_value());
  EXPECT_NE(result.error(), nullptr);
}

TEST(GetConnectionStringTest, ReturnsFormattedStringWhenAllFieldsAreSet) {
  ScopedEnvVar host{"POSTGRESQL_DB_HOST", "localhost"};
  ScopedEnvVar port{"POSTGRESQL_DB_PORT", "5432"};
  ScopedEnvVar name{"POSTGRESQL_DB_NAME", "spider"};
  ScopedEnvVar user{"POSTGRESQL_DB_USERNAME", "postgres"};
  ScopedEnvVar password{"POSTGRESQL_DB_PASSWORD", "secret"};

  const auto connectionString = getConnectionString();

  EXPECT_NE(connectionString.find("host=localhost"), std::string::npos);
  EXPECT_NE(connectionString.find("port=5432"), std::string::npos);
  EXPECT_NE(connectionString.find("dbname=spider"), std::string::npos);
  EXPECT_NE(connectionString.find("user=postgres"), std::string::npos);
  EXPECT_NE(connectionString.find("password=secret"), std::string::npos);
}

TEST(GetConnectionStringTest, ThrowsWhenAnyRequiredFieldIsMissing) {
  ScopedEnvVar host{"POSTGRESQL_DB_HOST", "localhost"};
  ScopedEnvVar port{"POSTGRESQL_DB_PORT", "5432"};
  ScopedEnvVar name{"POSTGRESQL_DB_NAME", "spider"};
  ScopedEnvVar user{"POSTGRESQL_DB_USERNAME", "postgres"};
  ScopedEnvVar password{"POSTGRESQL_DB_PASSWORD", ""};

  EXPECT_THROW(getConnectionString(), std::invalid_argument);
}

}  // namespace postgres_test
