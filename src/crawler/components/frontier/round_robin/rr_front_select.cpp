#include "rr_front_select.h"

namespace crawler {

namespace component {

std::optional<URL> RoundRobinFrontSelector::extract(FrontQueues& frontQueues) {
    std::unique_lock<std::mutex> lock(mutex_);

    const std::size_t nQueues = frontQueues.numQueues();

    for (std::size_t i = 0; i < nQueues; ++i) {
        const std::size_t currentIndex = pointer_;
        pointer_ = (pointer_ + 1) % nQueues;

        auto urlOpt = frontQueues.selectAndPop(currentIndex);
        if (urlOpt.has_value()) {
            return urlOpt;
        }
    }

    return std::nullopt;
}

}

}