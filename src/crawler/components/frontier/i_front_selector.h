#pragma once

#include "front_queues.h"

namespace crawler {

namespace component {

class IFrontSelector {
public:
    virtual std::optional<URL> extract(FrontQueues& frontQueues) = 0;
    virtual ~IFrontSelector() = default;
};

}

}