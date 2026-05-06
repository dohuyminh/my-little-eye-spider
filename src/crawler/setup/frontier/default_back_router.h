#pragma once

#include "types/url.h"

namespace crawler {

namespace setup {

class DefaultBackRouter {
 public:
  DefaultBackRouter(std::size_t numQueues);

  std::size_t routeURL(const types::URL& url);

 private:
  std::size_t numQueues_;
};

}  // namespace setup

}  // namespace crawler
