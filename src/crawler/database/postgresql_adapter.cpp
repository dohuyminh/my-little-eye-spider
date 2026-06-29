#include "postgresql_adapter.h"

#include <memory>
#include <mutex>
#include <stdexcept>

namespace crawler::database {

PostgreSQLConnectionPool::PostgreSQLConnectionPool(std::string_view connString,
                                                   std::size_t numConnections) {
  // number of connections must be non-zero
  if (numConnections == 0) {
    throw std::invalid_argument(
        "[PostgreSQLConnectionPool] Connection Pool must contain at least 1 "
        "connection");
  }

  // check database connection's health
  pqxx::connection healthCheckCon{connString.data()};
  if (!healthCheckCon.is_open()) {
    throw std::runtime_error("[PostgreSQLConnectionPool] Failed to connect to database");
  }

  for (std::size_t i{}; i < numConnections; ++i) {
    connections_.push(std::make_unique<pqxx::connection>(connString.data()));
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

}  // namespace crawler::database
