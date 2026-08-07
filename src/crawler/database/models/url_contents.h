#pragma once

#include "database/models/metadata.h"
#include "url.h"

namespace crawler::database {

struct URLContent : public PostgresModel {

  const URL& url;
  std::string compressedHTML;

  URLContent(const URL& url, std::string_view hashedContent);

  std::string postgresToTuple() const noexcept override;

  constexpr std::string_view postgresCorrespondingTable() const noexcept override {
    return "url_contents";
  }

};

static_assert(
    PostgresTupleRepresentable<URLContent>, 
    "[crawler::database::URLContent] Model cannot be represented with a PostgreSQL-compatible tuple"
);

static_assert(
    PostgresHasCorrespondingTable<URLContent>, 
    "[crawler::database::URLContent] Model does not have a corresponding table in target PostgreSQL database");

}
