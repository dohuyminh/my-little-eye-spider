#include "url.h"
#include <format>

namespace crawler::database {

URL::URL(
    std::string_view url, 
    std::chrono::time_point<std::chrono::utc_clock> discoveredAt, 
    std::chrono::time_point<std::chrono::utc_clock> lastCrawled) :
  id(boost::uuids::uuid()), 
  url(url), 
  discoveredAt(discoveredAt), 
  lastCrawled(lastCrawled)
{}

std::string URL::postgresToTuple() const noexcept {
  
  return std::format("('{}', '{}', '{:%Y-%m-%d %H:%M:%S}', '{:%Y-%m-%d %H:%M:%S}')", 
      boost::uuids::to_string(id), 
      url, 
      discoveredAt, 
      lastCrawled);

}

}
