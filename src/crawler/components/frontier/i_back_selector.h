#pragma once

#include "back_queues.h"

namespace crawler {
    
namespace component {

class IBackSelector {
public:
    virtual std::optional<URL> extract(BackQueues& backQueues) = 0;
    virtual ~IBackSelector() = default;
};

}

}