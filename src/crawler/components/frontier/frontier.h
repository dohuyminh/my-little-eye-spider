#pragma once

#include "front_queues.h"
#include "back_queues.h"
#include "i_front_prioritizer.h"
#include "i_front_selector.h"
#include "i_back_router.h"
#include "i_back_selector.h"

#include <memory>

namespace crawler {

namespace component {

class Frontier {
public:

    Frontier(
        std::size_t numFrontQueues,
        std::size_t numBackQueues,
        IFrontPrioritizer* prioritizer, 
        IFrontSelector* frontSelector,
        IBackRouter* router,
        IBackSelector* backSelector
    );

    void insertToFrontQueue(const std::string& url);

    std::optional<URL> popFront();

    void insertToBackQueue(const std::vector<URL>& urls);

    std::optional<URL> popBack();

private:

    // queues access
    FrontQueues frontQueues_;
    BackQueues backQueues_;

    // for assigning each URL a respective priority to be put into front queue
    std::unique_ptr<IFrontPrioritizer> prioritizer_;

    // for selecting URL for back queue router
    std::unique_ptr<IFrontSelector> frontSelector_; 

    // for assigning URLs into back queue 
    std::unique_ptr<IBackRouter> router_;

    // for rate-limiting and URL extraction (results to be handed to)
    std::unique_ptr<IBackSelector> backSelector_;
};

}

}