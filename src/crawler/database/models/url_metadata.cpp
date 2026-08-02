#include "url_metadata.h"
#include <format>

namespace crawler::database {

URLMetadata::URLMetadata(
  const URL& url,
  std::string_view contentHash, 
  std::string_view simHash, 
  std::size_t contentLength
) : 
  url(url), 
  contentHash(contentHash), 
  simHash(simHash), 
  contentLength(contentLength)
{}

std::string URLMetadata::postgresToTuple() const noexcept {

  return std::format("({}, decode('{}', 'base64'), decode('{}', 'base64'), {})", 
      boost::uuids::to_string(url.id),
      contentHash, 
      simHash, 
      contentLength
  );

}

}
