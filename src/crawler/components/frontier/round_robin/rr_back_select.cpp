#include "rr_back_select.h"

namespace crawler {

namespace components {

std::optional<types::URL> RoundRobinBackSelector::extract(BackQueues& backQueues) {
    std::unique_lock<std::mutex> lock(mutex_);

    const std::size_t nQueues = backQueues.numQueues();

    for (std::size_t i = 0; i < nQueues; ++i) {
        const std::size_t currentIndex = pointer_;
        pointer_ = (pointer_ + 1) % nQueues;

        auto urlOpt = backQueues.selectAndPop(currentIndex);
        if (urlOpt.has_value()) {
            return urlOpt;
        }
    }

    return std::nullopt;
}

}

}