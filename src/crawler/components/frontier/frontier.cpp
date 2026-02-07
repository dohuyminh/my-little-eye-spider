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
        IBackSelector* backSelector,
        std::size_t batchSize
    ) : 
    sharedURLQueue_(sharedURLQueue),
    frontQueues_(numFrontQueues),
    backQueues_(numBackQueues), 
    prioritizer_(prioritizer), 
    frontSelector_(frontSelector),
    router_(router), 
    backSelector_(backSelector),
    batchSize_(batchSize > 0 ? batchSize : 1)
{}

void Frontier::insertToFrontQueue(const std::string& url) {
    try {
        types::URL urlObj(url);
        auto [ urlWithPriority, queueIndex ] = prioritizer_->selectQueue(url);
        frontQueues_.insert(std::move(urlWithPriority), queueIndex);
    } catch (const std::invalid_argument& e) {
        // Ignore invalid URL
    }
}

void Frontier::insertToBackQueue(const std::vector<types::URL>& urls) {
    for (const types::URL& url: urls) {
        std::size_t backQueueIndex = router_->routeURL(url);
        backQueues_.insert(url, backQueueIndex);
    }
}

void Frontier::insertToBackQueue(std::vector<types::URL>&& urls) {
    for (auto& url : urls) {
        std::size_t backQueueIndex = router_->routeURL(url);
        backQueues_.insert(std::move(url), backQueueIndex);
    }
}

void Frontier::insertToBackQueue(types::URL&& url) {
    std::size_t backQueueIndex = router_->routeURL(url);
    backQueues_.insert(std::move(url), backQueueIndex);
}

std::optional<types::URL> Frontier::popFront() {
    return frontSelector_->extract(frontQueues_);
}

std::vector<types::URL> Frontier::popFrontBatch(std::size_t maxCount) {
    return frontSelector_->extractBatch(frontQueues_, maxCount);
}

std::optional<types::URL> Frontier::popBack() {
    return backSelector_->extract(backQueues_);
}

std::vector<types::URL> Frontier::popBackBatch(std::size_t maxCount) {
    return backSelector_->extractBatch(backQueues_, maxCount);
}

void Frontier::runImpl() {
    // Process front queues → back queues (single batch extract, up to batchSize_)
    auto frontBatch = popFrontBatch(batchSize_);
    if (!frontBatch.empty()) {
        insertToBackQueue(std::move(frontBatch));
    }

    // Process back queues → shared queue for workers (single batch extract, up to batchSize_)
    auto backBatch = popBackBatch(batchSize_);
    for (auto& url : backBatch) {
        sharedURLQueue_->push(std::move(url));
    }
}

}

}