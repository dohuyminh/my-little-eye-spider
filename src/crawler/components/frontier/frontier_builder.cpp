#include "frontier_builder.h"
#include <stdexcept>

namespace crawler {

namespace components {

void FrontierBuilder::setProducingQueue(std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> producingQueue) {
    if (!producingQueue) {
        throw std::invalid_argument("Shared URL Queue must be a valid shared pointer to a moodycamel::ConcurrentQueue<types::URL> object");
    }
    producingQueue_ = producingQueue;
}

void FrontierBuilder::setConsumingQueue(std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingQueue) {
    if (!consumingQueue) {
        throw std::invalid_argument("Shared URL Queue must be a valid shared pointer to a moodycamel::ConcurrentQueue<types::URL> object");
    }
    consumingQueue_ = consumingQueue;
}

void FrontierBuilder::setFrontQueuesSize(std::size_t frontQueuesSize) {
    if (frontQueuesSize == 0) {
        throw std::invalid_argument("Frontier initialization requires at least 1 queue at the front");
    }
    frontQueuesSize_ = frontQueuesSize;
}

void FrontierBuilder::setBackQueuesSize(std::size_t backQueuesSize) {
    if (backQueuesSize == 0) {
        throw std::invalid_argument("Frontier initialization requires at least 1 queue at the back");
    }
    backQueuesSize_ = backQueuesSize;
}

void FrontierBuilder::setBatchSize(std::size_t batchSize) {
    batchSize_ = batchSize > 0 ? batchSize : 1;
}

void FrontierBuilder::setPrioritizer(IFrontPrioritizer* prioritizer) {
    if (!prioritizer) {
        throw std::invalid_argument("(Front) Prioritizer must be a valid heap-allocated object, to be owned by the frontier");
    }
    prioritizer_ = prioritizer;
}

void FrontierBuilder::setFrontSelector(IFrontSelector* selector) {
    if (!selector) {
        throw std::invalid_argument("Front Selector must be a valid heap-allocated object, to be owned by the frontier");
    }
    frontSelector_ = selector;
}

void FrontierBuilder::setRouter(IBackRouter* router) {
    if (!router) {
        throw std::invalid_argument("(Back) Router must be a valid heap-allocated object, to be owned by the frontier");
    }
    router_ = router;
}

void FrontierBuilder::setBackSelector(IBackSelector* selector) {
    if (!selector) {
        throw std::invalid_argument("Back Selector must be a valid heap-allocated object, to be owned by the frontier");
    }
    backSelector_ = selector;
}

std::unique_ptr<Frontier> FrontierBuilder::get() {
    if (!canBuild()) {
        throw std::runtime_error("Not every components of frontier has been provided");
    }

    auto frontier = std::make_unique<Frontier>(
        producingQueue_,
        consumingQueue_,
        frontQueuesSize_, backQueuesSize_, 
        prioritizer_, frontSelector_, 
        router_, backSelector_,
        batchSize_
    );

    producingQueue_ = nullptr;
    consumingQueue_ = nullptr;
    frontQueuesSize_ = backQueuesSize_ = 0;
    batchSize_ = 1;
    prioritizer_ = nullptr; 
    frontSelector_ = nullptr; 
    router_ = nullptr;
    backSelector_ = nullptr;

    return frontier;
}

}

}