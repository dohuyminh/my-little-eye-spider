#include "frontier.h"

#include <format>

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

}

}