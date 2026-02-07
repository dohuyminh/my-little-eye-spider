#pragma once

#include "back_queues.h"

#include <vector>

namespace crawler {

namespace components {

class IBackSelector {
public:
    virtual std::optional<types::URL> extract(BackQueues& backQueues) = 0;
    virtual std::vector<types::URL> extractBatch(BackQueues& backQueues, std::size_t maxCount) = 0;
    virtual ~IBackSelector() = default;
};

}

}