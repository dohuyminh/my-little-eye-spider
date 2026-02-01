#pragma once

#include "../i_front_selector.h"
#include "../front_queues.h"
#include <mutex>

namespace crawler {

namespace components {

class RoundRobinFrontSelector : public IFrontSelector {
public:
    RoundRobinFrontSelector() = default;    

    std::optional<types::URL> extract(FrontQueues& frontQueues) override;

private:
    std::size_t pointer_{ 0 };

    std::mutex mutex_;
};

}

}
