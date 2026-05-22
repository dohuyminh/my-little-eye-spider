#pragma once

#include "setup/frontier/default_frontier_return_type.h"

namespace crawler {

namespace setup {

class DefaultBackRouter {
 public:
  DefaultBackRouter(std::size_t numQueues);

  std::size_t routeURL(const DefaultFrontierReturnType& url);

 private:
  std::size_t numQueues_;
};

}  // namespace setup

}  // namespace crawler
