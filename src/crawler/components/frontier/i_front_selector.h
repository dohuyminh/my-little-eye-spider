#pragma once

#include "front_queues.h"

namespace crawler {

namespace components {

class IFrontSelector {
public:
    virtual std::optional<types::URL> extract(FrontQueues& frontQueues) = 0;
    virtual ~IFrontSelector() = default;
};

}

}