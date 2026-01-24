#pragma once

#include <cstddef>
#include <vector>
#include <optional>
#include <stdexcept>
#include <format>
#include <moodycamel/concurrentqueue.h>

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

    std::size_t numURLsInQueue(std::size_t queueIndex) const {
        if (queueIndex >= frontQueues_.size()) {
            throw std::out_of_range(std::format("Indexing queue of order {} from a set of {} queues", queueIndex, frontQueues_.size()));
        }

        // Note: Lock-free queues don't provide accurate size (would require lock)
        // This is intentional for performance - use only if necessary
        return frontQueues_[queueIndex].data.size_approx();
    }
 
private:
    
    struct QueueBucket {
        moodycamel::ConcurrentQueue<URL> data;
    };

    std::vector<QueueBucket> frontQueues_;

};

}

}