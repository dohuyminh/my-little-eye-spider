#include <gtest/gtest.h>
#include <format>
#include <memory>

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

TEST_F(PSQLTestDatabase, dummy) {

}

};
