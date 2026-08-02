#pragma once

#include <boost/uuid.hpp>
#include <cstdint>
#include "database/models/metadata.h"

namespace crawler::database {

struct RobotsTxtEntry : PostgresModel {

  const boost::uuids::uuid id; 
  std::string domain;
  std::string category;
  std::string relativePath{};
  std::uint64_t crawlDelay{0}; 

  RobotsTxtEntry(
      std::string_view domain, 
      std::string_view category, 
      std::string_view relativePath, 
      std::uint64_t crawlDelay);

  std::string postgresToTuple() const noexcept override; 

  constexpr std::string_view postgresCorrespondingTable() const noexcept override {
    return "robots_txt";
  }

};

static_assert(
    PostgresTupleRepresentable<RobotsTxtEntry>, 
    "[crawler::database::RobotsTxtEntry] Model cannot be represented with a PostgreSQL-compatible tuple");

static_assert(
    PostgresHasCorrespondingTable<RobotsTxtEntry>, 
    "[crawler::database::RobotsTxtEntry] Model does not have a corresponding table in target PostgreSQL database");

}
