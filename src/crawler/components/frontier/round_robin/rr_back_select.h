#pragma once

#include "../i_back_selector.h"
#include "../back_queues.h"

namespace crawler {

namespace component {

class RoundRobinBackSelector : public IBackSelector {
public:
    RoundRobinBackSelector() = default;    

    std::optional<URL> extract(BackQueues& backQueues) override;

private:
    std::size_t pointer_{ 0 };

    std::mutex mutex_;
};

}

}