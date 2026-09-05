#pragma once

#include <cstdint>
#include <string_view>
#include "database/postgresql_work_queue.h"

namespace crawler::setup {

class URLInserter {
public:
  
  enum class InsertStatus : std::uint8_t {
    INSERT_SUCCESS, 
    INSERT_CONTENT_COLLISION,
    INSERT_URL_COLLISION,
    INSERT_NETWORK_ERROR
  };

  explicit URLInserter();

  [[nodiscard]] InsertStatus insertURL(std::string_view url, std::span<const std::string> outlinks) noexcept;

private:

  void populateURLsAndData(std::string_view url, std::span<const std::string> outlinks) noexcept;

  std::shared_ptr<database::URL> getOrInsertURL(
      std::string_view url, 
      std::chrono::time_point<std::chrono::utc_clock> time = std::chrono::utc_clock::now());

  
  
private:
  database::PostgreSQLWorkQueue queue_;
};

}
