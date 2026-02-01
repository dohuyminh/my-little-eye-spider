#pragma once

#include "../i_back_selector.h"
#include "../back_queues.h"

namespace crawler {

namespace components {

class RoundRobinBackSelector : public IBackSelector {
public:
    RoundRobinBackSelector() = default;    

    std::optional<types::URL> extract(BackQueues& backQueues) override;

private:
    std::size_t pointer_{ 0 };

    std::mutex mutex_;
};

}

}