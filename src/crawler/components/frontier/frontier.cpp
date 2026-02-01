#include "frontier.h"

namespace crawler {

namespace components {

Frontier::Frontier(
        std::shared_ptr<services::pattern::SharedQueue<types::URL>> sharedURLQueue,
        std::size_t numFrontQueues,
        std::size_t numBackQueues,
        IFrontPrioritizer* prioritizer, 
        IFrontSelector* frontSelector,
        IBackRouter* router,
        IBackSelector* backSelector
    ) : 
    sharedURLQueue_(sharedURLQueue),
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

void Frontier::insertToBackQueue(const std::vector<types::URL>& urls) {
    for (const types::URL& url: urls) {
        std::size_t backQueueIndex = router_->routeURL(url);
        backQueues_.insert(url, backQueueIndex);
    }
}

std::optional<types::URL> Frontier::popFront() {
    return frontSelector_->extract(frontQueues_);
}

std::optional<types::URL> Frontier::popBack() {
    return backSelector_->extract(backQueues_);
}

void Frontier::runImpl() {
    // Process front queues to back queues
    auto frontURLOpt = popFront();
    if (frontURLOpt.has_value()) {
        types::URL frontURL = frontURLOpt.value();
        
        // Route to back queue(s)
        std::vector<types::URL> routedURLs{ frontURL };
        insertToBackQueue(routedURLs);
    }

    // Process back queues to shared queue for workers
    auto backURLOpt = popBack();
    if (backURLOpt.has_value()) {
        types::URL backURL = backURLOpt.value();
        sharedURLQueue_->push(backURL);
    }
}

}

}