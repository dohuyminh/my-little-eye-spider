#include "rr_router.h"

namespace crawler {

namespace components {

RoundRobinBackRouter::RoundRobinBackRouter(std::size_t numQueues) : 
    numQueues_(numQueues) 
{}

std::size_t RoundRobinBackRouter::routeURL(const types::URL& url) {
    std::size_t selectedQueue;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        selectedQueue = pointer_;
        pointer_ = (pointer_ + 1) % numQueues_;
    }

    return selectedQueue;
}

}

}