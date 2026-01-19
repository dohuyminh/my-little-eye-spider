#pragma once

#include "types/url.h"

#include <vector>
#include <queue>
#include <optional>

namespace crawler {

namespace component {

class BackQueues {
public:
    
    BackQueues(std::size_t numQueues);

    std::optional<URL> selectAndPop(std::size_t queueIndex);

    void insert(const URL& url, std::size_t queueIndex);

    void insert(URL&& url, std::size_t queueIndex);

private:
    std::vector<std::queue<URL>> backQueues_;
};

}

}