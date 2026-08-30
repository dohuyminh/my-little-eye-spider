#pragma once

#include "database/models/metadata.h"
#include "database/models/url.h"

namespace crawler::database {
  
struct URLOutlink : public PostgresModel {
  
  std::shared_ptr<const URL> srcURL;
  std::shared_ptr<const URL> destURL;

  URLOutlink(std::shared_ptr<const URL> srcURL, std::shared_ptr<const URL> destURL);

  std::string postgresToTuple() const noexcept override;

  constexpr std::string_view postgresCorrespondingTable() const noexcept override {
    return "url_outlinks";
  }

};

static_assert(
    PostgresTupleRepresentable<URLOutlink>, 
    "[crawler::database::URLOutlink] Model cannot be represented with a PostgreSQL-compatible tuple");

static_assert(
    PostgresHasCorrespondingTable<URLOutlink>, 
    "[crawler::database::URLOutlink] Model does not have a corresponding table in target PostgreSQL database");

}
