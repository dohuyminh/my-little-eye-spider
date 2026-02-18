#pragma once

#include <mutex>

#include "../i_back_router.h"

namespace crawler {

namespace components {

class RoundRobinBackRouter : public IBackRouter {
 public:
  RoundRobinBackRouter(std::size_t numQueues);

  std::size_t routeURL(const types::URL& url) override;

 private:
  std::size_t numQueues_;
  std::size_t pointer_{0};

  std::mutex mutex_;
};

}  // namespace components

}  // namespace crawler