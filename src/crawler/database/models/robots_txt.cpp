#include "robots_txt.h"
#include <format>
#include <stdexcept>

namespace crawler::database {

RobotsTxtEntry::RobotsTxtEntry(
    std::string_view domain, 
    std::string_view category, 
    std::string_view relativePath, 
    std::uint64_t crawlDelay
) :
  id(boost::uuids::uuid()),
  domain(domain), 
  category(category), 
  relativePath(relativePath), 
  crawlDelay(crawlDelay) {
  
  // domain must not be empty
  if (domain.empty()) {
    throw std::invalid_argument("[crawler::database::RobotsTxtEntry] domain must not be empty");
  }

  // category must be one of the 3 valid options
  if (category != "Allow" && category != "Disallow" && category != "CrawlDelay") {
    throw std::invalid_argument("[crawler::database::RobotsTxtEntry] category must be one of the following: \"Allow\", \"Disallow\" or \"CrawlDelay\"" );
  }
}

std::string RobotsTxtEntry::postgresToTuple() const noexcept {
  
  return std::format("('{}', '{}', '{}', '{}', {})", 
      boost::uuids::to_string(id), 
      domain,
      category, 
      relativePath, 
      crawlDelay);

} 

}
