#include "postgresql_work.h"
#include <format>
#include <ranges>
#include <stdexcept>

namespace crawler::database {

using namespace std::literals;

PostgresDatabaseWork::PostgresDatabaseWork(std::string_view tableName) : 
  tableName_(tableName) 
{}

PostgresSelectWork::PostgresSelectWork(
    std::string_view tableName, 
    std::initializer_list<std::string> filters) : 
    PostgresDatabaseWork(tableName), 
    filters_(filters)
  {}


std::string PostgresSelectWork::toSQLCommand() const noexcept {
  
  std::string cmd{std::format("SELECT * FROM {}", tableName())};
  if (!filters_.empty()) {
    cmd += " WHERE ";
    for (std::size_t i{0}; i < filters_.size(); ++i) {
      cmd += filters_[i];
      if (i < filters_.size() - 1) [[likely]] {
        cmd += " AND ";
      }
    }
  }

  cmd.push_back(';');

  return cmd;
}

PostgresInsertWork::PostgresInsertWork(
    std::string_view tableName, 
    std::initializer_list<Model> models) :
    PostgresDatabaseWork(tableName), 
    models_(models)
{
  // ensure all models are of the same type
  for (const Model& m: models_) {
    auto& underlyingModel = getUnderlyingModel<PostgresModel>(m);
    if (this->tableName() != underlyingModel.postgresCorrespondingTable()) {
      throw std::invalid_argument(std::format("[PostgresInsertWork] model with data {} is incompatible to insert to table \"{}\"", 
            underlyingModel.postgresToTuple(), this->tableName()));
    }
  }
}

void PostgresInsertWork::insertRowIntoBatch(std::span<Model> models) {
  
  // ensure all models are of the same type
  for (const Model& m: models) {
    auto& underlyingModel = getUnderlyingModel<PostgresModel>(m);
    if (tableName() != underlyingModel.postgresCorrespondingTable()) {
      throw std::invalid_argument(std::format("[PostgresInsertWork] model with data {} is incompatible to insert to table \"{}\"", 
            underlyingModel.postgresToTuple(), tableName()));
    }
  }

  for (const auto& m: models) {
    models_.push_back(m);
  }
} 

std::string PostgresInsertWork::toSQLCommand() const {
  if (models_.empty()) {
    throw std::runtime_error("[PostgresInsertWork] No value to insert to table");
  }

  std::string cmd{std::format("INSERT INTO {} VALUES ", tableName())};
  for (std::size_t i{0}; i < models_.size(); ++i) {
    cmd += getUnderlyingModel<PostgresModel>(models_[i]).postgresToTuple();
    if (i < models_.size() - 1) [[likely]] {
      cmd.push_back(',');
    }
  } 

  cmd.push_back(';');

  return cmd;
}

PostgresUpdateWork::PostgresUpdateWork(
    std::string_view tableName, 
    std::initializer_list<std::string> filters, 
    std::initializer_list<std::string> updateValues) : 
  PostgresDatabaseWork{tableName}, 
  filters_{filters}, 
  updateValues_{updateValues}
{
  if (updateValues_.empty()) {
    throw std::invalid_argument("[PostgresUpdateWork] Update work must contain at least 1 column update");
  }
}

std::string PostgresUpdateWork::toSQLCommand() const {
  
  std::string cmd{std::format("UPDATE {} SET ", tableName())};

  for (const auto& [i, update]: updateValues_ | std::views::enumerate) {
    cmd += update;
    if (static_cast<std::size_t>(i) < updateValues_.size() - 1) [[likely]] {
      cmd.push_back(',');
    }
  } 

  
  if (!filters_.empty()) {
    cmd += " WHERE ";
    for (const auto& [i, update]: updateValues_ | std::views::enumerate) {
      cmd += update;
      if (static_cast<std::size_t>(i) < filters_.size() - 1) [[likely]] {
        cmd += " AND ";
      }
    }
  }


  return cmd;
}

}
