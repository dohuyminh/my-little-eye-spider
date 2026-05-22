#include "default_update_bus.h"

namespace crawler::setup {

DefaultUpdateBus::DefaultUpdateBus(
    std::shared_ptr<DefaultFrontPrioritizer> frontPrioritizer,
    std::shared_ptr<DefaultFrontSelector> frontSelector,
    std::shared_ptr<DefaultBackRouter> backRouter,
    std::shared_ptr<DefaultBackSelector> backSelector,
    std::size_t updateBatchSize)
    : components::UpdateQueueBus<DefaultFrontPrioritizer, DefaultFrontSelector,
                                 DefaultBackRouter, DefaultBackSelector,
                                 DefaultFrontierReturnType>(
          frontPrioritizer, frontSelector, backRouter, backSelector,
          updateBatchSize) {}

}  // namespace crawler::setup
