#include "back_queues.h"

#include <stdexcept>
#include <format>

namespace crawler {

namespace component {

BackQueues::BackQueues(std::size_t numQueues) : 
    backQueues_(numQueues) 
{
    if (numQueues == 0) {
        throw std::invalid_argument("BackQueues must have at least 1 bucket");
    }
}

std::optional<URL> BackQueues::selectAndPop(std::size_t queueIndex) {
    if (queueIndex >= backQueues_.size()) {
        throw std::out_of_range(
            std::format("Accessing the {}th queue from a set of {} queues", 
                       queueIndex, backQueues_.size())
        );
    }

    URL url;
    
    // Lock-free dequeue: O(1) time complexity
    // No mutexes: Multiple threads can dequeue simultaneously
    // Thread-safe: Uses atomic operations internally
    if (backQueues_[queueIndex].data.try_dequeue(url)) {
        return url;
    }
    
    return {};
}

void BackQueues::insert(const URL& url, std::size_t queueIndex) {
    if (queueIndex >= backQueues_.size()) {
        throw std::out_of_range(
            std::format("Accessing the {}th queue from a set of {} queues", 
                       queueIndex, backQueues_.size())
        );
    }
    
    // Lock-free enqueue: O(1) time complexity
    // No mutexes: Multiple threads can enqueue simultaneously (MPMC)
    // Thread-safe: Uses atomic operations internally
    backQueues_[queueIndex].data.enqueue(url);
}

void BackQueues::insert(URL&& url, std::size_t queueIndex) {
    if (queueIndex >= backQueues_.size()) {
        throw std::out_of_range(
            std::format("Accessing the {}th queue from a set of {} queues", 
                       queueIndex, backQueues_.size())
        );
    }
    
    // Lock-free move enqueue: O(1) time complexity
    // Optimized: Moves URL instead of copying
    // No mutexes: Multiple threads can enqueue simultaneously (MPMC)
    backQueues_[queueIndex].data.enqueue(std::move(url));
}

}

}