#include "front_queues.h"

#include <stdexcept>
#include <format>

namespace crawler {

namespace component {

FrontQueues::FrontQueues(std::size_t n_queues) :
    frontQueues_(n_queues) 
{}

std::optional<URL> FrontQueues::selectAndPop(std::size_t queueIndex) {
    if (queueIndex >= frontQueues_.size()) {
        throw std::out_of_range(std::format("Accessing the {}th queue from a set of {} queues", queueIndex, frontQueues_.size()));
    }

    URL url;
    
    // Lock-free dequeue: O(1) time complexity
    // No mutexes: Multiple threads can dequeue simultaneously
    // Thread-safe: Uses atomic operations internally
    if (frontQueues_[queueIndex].data.try_dequeue(url)) {
        return url;
    }
    
    return {};
}

void FrontQueues::insert(const URL& url, std::size_t queueIndex) {
    if (queueIndex >= frontQueues_.size()) {
        throw std::out_of_range(std::format("Accessing the {}th queue from a set of {} queues", queueIndex, frontQueues_.size()));
    }
    
    // Lock-free enqueue: O(1) time complexity
    // No mutexes: Multiple threads can enqueue simultaneously (MPMC)
    // Thread-safe: Uses atomic operations internally
    frontQueues_[queueIndex].data.enqueue(url);
}

void FrontQueues::insert(URL&& url, std::size_t queueIndex) {
    if (queueIndex >= frontQueues_.size()) {
        throw std::out_of_range(std::format("Accessing the {}th queue from a set of {} queues", queueIndex, frontQueues_.size()));
    }
    
    // Lock-free move enqueue: O(1) time complexity
    // Optimized: Moves URL instead of copying
    // No mutexes: Multiple threads can enqueue simultaneously (MPMC)
    frontQueues_[queueIndex].data.enqueue(std::move(url));
}

}

}