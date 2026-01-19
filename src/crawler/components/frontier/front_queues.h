#pragma once

#include <queue>
#include <cstddef>
#include <vector>
#include <optional>

#include "types/url.h"

namespace crawler {

namespace component {

class FrontQueues {
public:

    FrontQueues(std::size_t nQueues);

    std::optional<URL> selectAndPop(std::size_t queueIndex);

    void insert(const URL& url, std::size_t queueIndex);

    void insert(URL&& url, std::size_t queueIndex);

    std::size_t numQueues() const noexcept {
        return frontQueues_.size();
    }
 
private:
    
    std::vector<std::priority_queue<URL>> frontQueues_;

};

}

}