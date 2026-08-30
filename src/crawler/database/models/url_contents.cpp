#include "database/models/url_contents.h"
#include <format>

namespace crawler::database {

URLContent::URLContent(std::shared_ptr<const URL> url, std::string_view compressedHTML) : 
  url(url),
  compressedHTML(compressedHTML)
{}

std::string URLContent::postgresToTuple() const noexcept {
  
  return std::format("('{}', decode('{}', 'base64'))", boost::uuids::to_string(url->id), compressedHTML);

}

}
