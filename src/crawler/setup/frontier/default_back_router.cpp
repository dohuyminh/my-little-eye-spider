#include "default_back_router.h"

namespace crawler::setup {

DefaultBackRouter::DefaultBackRouter(std::size_t numQueues)
    : numQueues_(numQueues) {
  if (numQueues_ == 0) {
    throw std::invalid_argument("Number of queues must be positive");
  }
}

std::size_t DefaultBackRouter::routeURL(const types::URL& url) {
  return std::hash<std::string>{}(url.domain()) % numQueues_;
}

}  // namespace crawler::setup
