#pragma once

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <pqxx/pqxx>
#include <queue>
#include <thread>
#include <utility>

#include "database_adapter.h"

namespace crawler::database {

class PostgreSQLConnectionPool {
 public:
  using ConnectionPtr = std::unique_ptr<pqxx::connection>;

  class ConnectionLease {
   public:
    ConnectionLease(pqxx::connection* conn, PostgreSQLConnectionPool& pool);

    ~ConnectionLease();

    pqxx::connection& operator*() noexcept { return *conn_; }

    pqxx::connection* operator->() noexcept { return conn_; }

   private:
    pqxx::connection* conn_;
    PostgreSQLConnectionPool& pool_;
  };

  PostgreSQLConnectionPool(std::string_view connString,
                           std::size_t numConnections);

  ConnectionLease acquireConnection();

  void releaseConnection(pqxx::connection* conn);

 private:
  std::queue<ConnectionPtr> connections_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

class PostgreSQLAdapter {
 public:
  PostgreSQLAdapter(
      std::string_view connectionString,
      std::size_t numConnections = std::thread::hardware_concurrency() * 2);

  template <typename... Args>
  DbQueryReturn<pqxx::result> execute(std::string_view query, Args&&... args) {
    auto conn{pool_.acquireConnection()};
    pqxx::work tx{*conn};

    try {
      auto r = tx.exec(query, std::forward<Args>(args)...);
      tx.commit();
      return r;
    } catch (...) {
      return std::unexpected(ExecutionStatus::EXEC_FAIL);
    }
  }

 private:
  PostgreSQLConnectionPool pool_;
};

}  // namespace crawler::database
