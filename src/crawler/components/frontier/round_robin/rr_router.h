#pragma once

#include "../i_back_router.h"
#include <mutex>

namespace crawler {

namespace components {

class RoundRobinBackRouter : public IBackRouter {
public:
    RoundRobinBackRouter(std::size_t numQueues);    

    std::size_t routeURL(const types::URL& url) override;

private:
    std::size_t numQueues_;
    std::size_t pointer_{ 0 };

    std::mutex mutex_;
};

}

}