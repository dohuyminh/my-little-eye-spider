#pragma once

#include <initializer_list>
#include <pqxx/pqxx>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "entities.h"
#include "models/models.h"

namespace crawler::database {
  
class PostgresDatabaseWork {
public:
  
  PostgresDatabaseWork(std::string_view tableName);

  std::string_view tableName() const noexcept {
    return tableName_;
  }

  ResultAsync<pqxx::result>& returnData() noexcept {
    return returnData_;
  }

  virtual std::string toSQLCommand() const = 0;

  virtual ~PostgresDatabaseWork() = default;

private:
  std::string tableName_;
  ResultAsync<pqxx::result> returnData_;
};


class PostgresSelectWork : public PostgresDatabaseWork {
public:
  
  PostgresSelectWork(
      std::string_view tableName, 
      std::initializer_list<std::string> filters);

  std::span<const std::string> filters() noexcept {
    return std::span<const std::string>{filters_};
  }

  std::string toSQLCommand() const noexcept override; 

private:
  std::vector<std::string> filters_;

};


class PostgresInsertWork : public PostgresDatabaseWork {
public:

  PostgresInsertWork(
      std::string_view tableName, 
      std::initializer_list<Model> models);

  void insertRowIntoBatch(std::span<Model> rows);

  std::string toSQLCommand() const override; 

private:
  std::vector<Model> models_;
  
};


class PostgresUpdateWork : public PostgresDatabaseWork {
public:

  PostgresUpdateWork(
      std::string_view tableName, 
      std::initializer_list<std::string> filters,
      std::initializer_list<std::string> updateValues);

  std::string toSQLCommand() const override;

private:
  std::vector<std::string> filters_;
  std::vector<std::string> updateValues_;
};


class PostgresDeleteWork : public PostgresDatabaseWork {

};

}
