#include <gtest/gtest.h>
#include <exception>
#include <format>
#include <memory>
#include <print>

#include "crawler/database/postgresql_adapter.h"
#include "dotenv.h"

using namespace crawler::database;

namespace postgres_test {

// set up a test database with custom credentials
class PSQLTestDatabase : public ::testing::Test {
protected:
  std::unique_ptr<PostgreSQLAdapter> adapter{ nullptr };

  PSQLTestDatabase() = default; 

  virtual ~PSQLTestDatabase() = default;

  virtual void SetUp() override {
    
    constexpr char testdbName[] = "testdb";
    
    pqxx::connection postgresConn{getTestConnectionString("postgres")};
    
    // check if test database exists
    pqxx::nontransaction w{postgresConn}; 
    auto res = w.exec(
        std::format("SELECT datname FROM pg_catalog.pg_database WHERE datname = '{}'", testdbName)
    );

    if (res.size() == 0) {
      w.exec(std::format("CREATE DATABASE {}", testdbName));
    } 

    adapter = std::make_unique<PostgreSQLAdapter>(getTestConnectionString(testdbName));
  }

  virtual void TearDown() override {
    adapter.reset();
  } 

private:
  std::string getTestConnectionString(std::string_view databaseName) const {

    dotenv::env.load_dotenv("../../.env");

    if (dotenv::env["POSTGRESQL_DB_HOST"].empty()) [[unlikely]] {
      throw std::invalid_argument("[getTestConnectionString] PostgreSQL Host is not configured");
    }

    if (dotenv::env["POSTGRESQL_DB_PORT"].empty()) [[unlikely]] {
      throw std::invalid_argument("[getTestConnectionString] PostgreSQL Port is not configured");
    }
    
    if (dotenv::env["POSTGRESQL_DB_USERNAME"].empty()) [[unlikely]] {
      throw std::invalid_argument("[getTestConnectionString] PostgreSQL Username is not configured");
    }

    if (dotenv::env["POSTGRESQL_DB_PASSWORD"].empty()) [[unlikely]] {
      throw std::invalid_argument("[getTestConnectionString] PostgreSQL Password is not configured");
    }

    return std::format("host={} port={} dbname={} user={} password={}", 
        dotenv::env["POSTGRESQL_DB_HOST"],
        dotenv::env["POSTGRESQL_DB_PORT"],
        databaseName,
        dotenv::env["POSTGRESQL_DB_USERNAME"],
        dotenv::env["POSTGRESQL_DB_PASSWORD"]);
  }

};

class SingleThreadedSetting : public PSQLTestDatabase {
protected:
  
  void SetUp() override final {
    PSQLTestDatabase::SetUp();

    auto status = adapter->execute(
        "CREATE TABLE st_test_table ("
        " id BIGINT PRIMARY KEY, "
        " integer_val INT, " 
        " string_val VARCHAR(225), "
        " unique_int_val INT UNIQUE NOT NULL);", 
        true);
      
    if (!status.has_value()) {
      std::rethrow_exception(status.error());
    }
  } 
  
  void TearDown() override final {
    auto status = adapter->execute("DROP TABLE st_test_table", true); 
    if (!status.has_value()) {
      std::rethrow_exception(status.error());
    }
    
    PSQLTestDatabase::TearDown();
  }
};

// Test adapter on single-threaded setting (1 writer + 1 reader)
TEST_F(SingleThreadedSetting, TestAdapterForSingleThreadedSetting) {
  
  // valid writes
  auto status1 = adapter->execute(
      std::format("INSERT INTO st_test_table VALUES ({}, {}, '{}', {});", 
        1, 10, "Hello", 2), 
      true);
  auto status2 = adapter->execute(
      std::format("INSERT INTO st_test_table VALUES ({}, {}, '{}', {});", 
        2, 20, "World", 3), 
      true);

  ASSERT_TRUE(status1.has_value());
  ASSERT_TRUE(status2.has_value());
  
  EXPECT_TRUE(status1.value().affected_rows() == 1);
  EXPECT_TRUE(status2.value().affected_rows() == 1);

  // invalid writes 
  auto invalidStatus = adapter->execute(
      std::format("INSERT INTO st_test_table VALUES ({}, {}, '{}', {})", 
        3, 30, "Invalid", 3), 
      true);
      
  ASSERT_TRUE(!invalidStatus.has_value());

  // single selects
  auto readStatus1 = adapter->execute("SELECT * FROM st_test_table;");
  auto readStatus2 = adapter->execute("SELECT * FROM st_test_table WHERE integer_val > 10;");

  ASSERT_TRUE(readStatus1.has_value());
  ASSERT_TRUE(readStatus2.has_value());

  EXPECT_TRUE(readStatus1.value().size() == 2);
  EXPECT_TRUE(readStatus2.value().size() == 1);
}

// Test adapter on multi-threaded setting (concurrent readers)
// Test adapter on multi-threaded setting (concurrent writers; check for data integrity)
// Test adapter on multi-threaded setting (concurrent writers + concurrent readers; check for data integrity)

TEST_F(PSQLTestDatabase, dummy) {

}

};
