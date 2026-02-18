#include "hash_router.h"

namespace crawler {

namespace components {

HashBackRouter::HashBackRouter(std::size_t numBackQueues)
    : numBackQueues_(numBackQueues) {}

std::size_t HashBackRouter::routeURL(const types::URL& url) {
  std::hash<std::string> hasher;
  std::size_t hashValue = hasher(url.domain());
  return hashValue % numBackQueues_;
}

}  // namespace components

}  // namespace crawler