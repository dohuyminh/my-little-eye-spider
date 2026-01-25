#include "frontier_builder.h"
#include <stdexcept>

namespace crawler {

namespace component {

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
}

Frontier FrontierBuilder::get() {
    if (!canBuild()) {
        throw std::runtime_error("Not every component of frontier has been provided");
    }

    Frontier frontier(
        frontQueuesSize_, backQueuesSize_, 
        prioritizer_, frontSelector_, 
        router_, backSelector_
    );

    frontQueuesSize_ = backQueuesSize_ = 0;
    prioritizer_ = nullptr; 
    frontSelector_ = nullptr; 
    router_ = nullptr;
    backSelector_ = nullptr;

    return frontier;
}

}

}