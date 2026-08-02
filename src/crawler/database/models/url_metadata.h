#pragma once

#include "database/models/metadata.h"
#include "database/models/url.h"

namespace crawler::database {

struct URLMetadata : PostgresModel { 
  
  const URL& url;
  std::string contentHash;
  std::string simHash;
  std::size_t contentLength;

  URLMetadata(
      const URL& url, 
      std::string_view contentHash, 
      std::string_view simHash, 
      std::size_t contentLength
  );

  std::string postgresToTuple() const noexcept override;

  constexpr std::string_view postgresCorrespondingTable() const noexcept override {
    return "url_metadatas";
  } 

};

static_assert(
    PostgresTupleRepresentable<URLMetadata>, 
    "[crawler::database::URLMetadata] Model cannot be represented with a PostgreSQL-compatible tuple");

static_assert(
    PostgresHasCorrespondingTable<URLMetadata>, 
    "[crawler::database::URLMetadata] Model does not have a corresponding table in target PostgreSQL database");

}
