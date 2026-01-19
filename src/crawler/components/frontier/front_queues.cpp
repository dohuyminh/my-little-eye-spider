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

    std::priority_queue<URL>& pq = frontQueues_[queueIndex];

    if (pq.empty()) {
        return {};
    }        

    URL res = pq.top();
    pq.pop();

    return res;
}

void FrontQueues::insert(const URL& url, std::size_t queueIndex) {
    if (queueIndex >= frontQueues_.size()) {
        throw std::out_of_range(std::format("Accessing the {}th queue from a set of {} queues", queueIndex, frontQueues_.size()));
    }
    
    frontQueues_[queueIndex].push(url);
}

void FrontQueues::insert(URL&& url, std::size_t queueIndex) {
    if (queueIndex >= frontQueues_.size()) {
        throw std::out_of_range(std::format("Accessing the {}th queue from a set of {} queues", queueIndex, frontQueues_.size()));
    }
    
    frontQueues_[queueIndex].push(std::move(url));    
}

}

}