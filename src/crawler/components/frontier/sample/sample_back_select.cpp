#include "sample_back_select.h"

namespace crawler {

namespace components {

std::optional<types::URL> SampleBackSelector::extract(BackQueues& backQueues) {
    return backQueues.selectAndPop(0);
}

std::vector<types::URL> SampleBackSelector::extractBatch(BackQueues& backQueues, std::size_t maxCount) {
    std::vector<types::URL> result;
    result.reserve(maxCount);
    for (std::size_t i = 0; i < maxCount; ++i) {
        auto urlOpt = backQueues.selectAndPop(0);
        if (!urlOpt.has_value()) break;
        result.push_back(std::move(urlOpt.value()));
    }
    return result;
}

}

}