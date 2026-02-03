#pragma once

#include "back_queues.h"

namespace crawler {
    
namespace components {

class IBackSelector {
public:
    virtual std::optional<types::URL> extract(BackQueues& backQueues) = 0;
    virtual ~IBackSelector() = default;
};

}

}