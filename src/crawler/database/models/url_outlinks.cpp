#include "url_outlinks.h"
#include <format>

namespace crawler::database {

URLOutlink::URLOutlink(std::shared_ptr<const URL> srcURL, std::shared_ptr<const URL> destURL) : 
  srcURL(srcURL), 
  destURL(destURL)
{}

std::string URLOutlink::postgresToTuple() const noexcept {
  return std::format("('{}', '{}')", 
      boost::uuids::to_string(srcURL->id), 
      boost::uuids::to_string(destURL->id));
}

}
