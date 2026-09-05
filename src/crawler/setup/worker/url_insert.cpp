#include "url_insert.h"
#include <algorithm>
#include <chrono>
#include <memory>

#include "database/postgresql_adapter.h"
#include "database/postgresql_work.h"

namespace crawler::setup {

URLInserter::URLInserter() : 
  queue_{database::getConnectionString(), 20}
{}

URLInserter::InsertStatus URLInserter::insertURL(
    std::string_view url, 
    std::span<const std::string> outlinks) noexcept {

  auto time = std::chrono::utc_clock::now();
  
  // check if url already exists in database  
  auto checkExists = std::make_shared<database::PostgresSelectWork>(
    "urls",
    std::initializer_list<std::string>{std::format("url = '{}'", url)}
  );

  queue_.logWork(checkExists);
  auto checkResult = checkExists->returnData().getFuture().get();
  if (checkResult.size() > 0) {
    // update last visited and return URL collision flag (this will affect how the crawler is rewarded later)
    auto updateLastCrawled = std::make_shared<database::PostgresUpdateWork>(
      "urls",
      std::initializer_list<std::string>{std::format("url = '{}'", url)}, 
      std::initializer_list<std::string>{std::format("last_crawled = '{:%Y-%m-%d %H:%M:%S}'", time)}
    );
    queue_.logWork(updateLastCrawled);

    return InsertStatus::INSERT_URL_COLLISION;
  }

  // [TODO] check if the content is a duplicate of that of an existing url 

  // should inserts be async?
  // yes; all validation code has been done synchronously prior to insertions
  // any failures during insertions can only be network-related issues
  // frontier should not be punished because of network errors

  // insert into urls table entry  
  // insert edges in url_outlinks
  // insert metadata in url_contents and url_metadatas
  

  return InsertStatus::INSERT_SUCCESS;
}

}
