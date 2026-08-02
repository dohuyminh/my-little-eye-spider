#pragma once

#include "metadata.h"

#include <boost/uuid.hpp>
#include <chrono>

namespace crawler::database {
  
struct URL : PostgresModel {
  
  const boost::uuids::uuid id;
  const std::string url;
  const std::chrono::time_point<std::chrono::utc_clock> discoveredAt;
  const std::chrono::time_point<std::chrono::utc_clock> lastCrawled;

  URL(
      std::string_view url, 
      std::chrono::time_point<std::chrono::utc_clock> discoveredAt, 
      std::chrono::time_point<std::chrono::utc_clock> lastCrawled);

  std::string postgresToTuple() const noexcept override; 

  constexpr std::string_view postgresCorrespondingTable() const noexcept override {
    return "urls";
  }

};

}
