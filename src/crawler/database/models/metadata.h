#pragma once

#include <concepts>
#include <string>

namespace crawler::database {

class PostgresModel {
public:
  virtual std::string postgresToTuple() const noexcept = 0;
  virtual std::string_view postgresCorrespondingTable() const noexcept = 0;

  virtual ~PostgresModel() = default;
};

template <typename T>
concept PostgresTupleRepresentable = requires (T t) {
  { t.postgresToTuple() } -> std::same_as<std::string>;
};

template <typename T>
concept PostgresHasCorrespondingTable = requires (T t) {
  { t.postgresCorrespondingTable() } -> std::same_as<std::string_view>;
};

static_assert(
    PostgresTupleRepresentable<PostgresModel> && 
    PostgresHasCorrespondingTable<PostgresModel>);

};
