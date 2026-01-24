#include "frontier.h"

namespace crawler {

namespace component {

Frontier::Frontier(std::size_t numFrontQueues,
        std::size_t numBackQueues,
        IFrontPrioritizer* prioritizer, 
        IFrontSelector* frontSelector,
        IBackRouter* router,
        IBackSelector* backSelector
    ) : 
    frontQueues_(numFrontQueues),
    backQueues_(numBackQueues), 
    prioritizer_(prioritizer), 
    frontSelector_(frontSelector),
    router_(router), 
    backSelector_(backSelector)
{}

void Frontier::insertToFrontQueue(const std::string& url) {
    auto [ urlWithPriority, queueIndex ] = prioritizer_->selectQueue(url);
    frontQueues_.insert(urlWithPriority, queueIndex);
}

void Frontier::insertToBackQueue(const std::vector<URL>& urls) {
    for (const URL& url: urls) {
        std::size_t backQueueIndex = router_->routeURL(url);
        backQueues_.insert(url, backQueueIndex);
    }
}

std::optional<URL> Frontier::popFront() {
    return frontSelector_->extract(frontQueues_);
}

std::optional<URL> Frontier::popBack() {
    return backSelector_->extract(backQueues_);
}

}

}