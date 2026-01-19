#pragma once

#include "back_queues.h"

#include <stdexcept>
#include <format>

namespace crawler {

namespace component {

BackQueues::BackQueues(std::size_t numQueues) : 
    backQueues_(numQueues) 
{}

std::optional<URL> BackQueues::selectAndPop(std::size_t queueIndex) {
    if (queueIndex >= backQueues_.size()) {
        throw std::out_of_range(std::format("Accessing the {}th queue from a set of {} queues", queueIndex, backQueues_.size()));
    }     

    std::queue<URL>& pq = backQueues_[queueIndex];

    if (pq.empty()) {
        return {};
    }        

    URL res = pq.front();
    pq.pop();

    return res;
}

void BackQueues::insert(const URL& url, std::size_t queueIndex) {
    if (queueIndex >= backQueues_.size()) {
        throw std::out_of_range(std::format("Accessing the {}th queue from a set of {} queues", queueIndex, backQueues_.size()));
    }
    
    backQueues_[queueIndex].push(url);
}

void BackQueues::insert(URL&& url, std::size_t queueIndex) {
    if (queueIndex >= backQueues_.size()) {
        throw std::out_of_range(std::format("Accessing the {}th queue from a set of {} queues", queueIndex, backQueues_.size()));
    }
    
    backQueues_[queueIndex].push(std::move(url));    
}

}

}