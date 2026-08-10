#include "postgresql_adapter.h"

#include <exception>
#include <format>
#include <memory>
#include <mutex>
#include <stdexcept>

#include "dotenv.h"

namespace crawler::database {

PostgreSQLConnectionPool::PostgreSQLConnectionPool(std::string_view connString,
                                                   std::size_t numConnections) {
  // number of connections must be non-zero
  if (numConnections == 0) {
    throw std::invalid_argument(
        "[PostgreSQLConnectionPool] Connection Pool must contain at least 1 "
        "connection");
  }

  try {
    // check database connection's health
    pqxx::connection healthCheckCon{connString.data()};
    if (!healthCheckCon.is_open()) {
      throw std::runtime_error("[PostgreSQLConnectionPool] Failed to connect to database");
    }
  
    for (std::size_t i{}; i < numConnections; ++i) {
      connections_.push(std::make_unique<pqxx::connection>(connString.data()));
    }
  } catch (const std::exception& e) {
    throw std::runtime_error("[PostgreSQLConnectionPool] Failed to initialize connection pool: " + std::string(e.what()));
  }
}

PostgreSQLConnectionPool::ConnectionLease::ConnectionLease(
    pqxx::connection *conn, PostgreSQLConnectionPool &pool)
    : conn_(conn), pool_(pool) {}

PostgreSQLConnectionPool::ConnectionLease::~ConnectionLease() {
  pool_.releaseConnection(conn_);
}

PostgreSQLConnectionPool::ConnectionLease
PostgreSQLConnectionPool::acquireConnection() {
  std::unique_lock<std::mutex> lock(mutex_);

  cv_.wait(lock, [&]() { return !connections_.empty(); });

  auto conn{connections_.front().release()};
  connections_.pop();

  return ConnectionLease(conn, *this);
}

void PostgreSQLConnectionPool::releaseConnection(pqxx::connection *conn) {
  std::unique_lock<std::mutex> lock(mutex_);

  connections_.push(std::unique_ptr<pqxx::connection>(conn));

  cv_.notify_one();
}

PostgreSQLAdapter::PostgreSQLAdapter(std::string_view connectionString,
                                     std::size_t numConnections)
    : pool_(connectionString, numConnections) {}

DbQueryReturn<pqxx::result> PostgreSQLAdapter::execute(std::string_view query, bool isWrite) noexcept {
  auto conn = pool_.acquireConnection();
  pqxx::work tx{*conn};

  try {
    auto r = tx.exec(query);
    if (isWrite) {
      tx.commit();
    }
    return r;
  } catch (...) {
    return std::unexpected(std::current_exception());
  }
}


std::string getConnectionString() {
  
  if (dotenv::env["POSTGRESQL_DB_HOST"].empty()) [[unlikely]] {
    throw std::invalid_argument("[getConnectionString] PostgreSQL Host is not configured");
  }

  if (dotenv::env["POSTGRESQL_DB_PORT"].empty()) [[unlikely]] {
    throw std::invalid_argument("[getConnectionString] PostgreSQL Port is not configured");
  }
  
  if (dotenv::env["POSTGRESQL_DB_NAME"].empty()) [[unlikely]] {
    throw std::invalid_argument("[getConnectionString] PostgreSQL Database name is not configured");
  }

  if (dotenv::env["POSTGRESQL_DB_USERNAME"].empty()) [[unlikely]] {
    throw std::invalid_argument("[getConnectionString] PostgreSQL Username is not configured");
  }

  if (dotenv::env["POSTGRESQL_DB_PASSWORD"].empty()) [[unlikely]] {
    throw std::invalid_argument("[getConnectionString] PostgreSQL Password is not configured");
  }

  return std::format("host={} port={} dbname={} user={} password={}", 
      dotenv::env["POSTGRESQL_DB_HOST"],
      dotenv::env["POSTGRESQL_DB_PORT"],
      dotenv::env["POSTGRESQL_DB_NAME"],
      dotenv::env["POSTGRESQL_DB_USERNAME"],
      dotenv::env["POSTGRESQL_DB_PASSWORD"]);
}

}  // namespace crawler::database
