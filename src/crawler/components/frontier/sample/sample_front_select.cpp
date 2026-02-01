#include "sample_front_select.h"

namespace crawler {

namespace components {

std::optional<types::URL> SampleFrontSelector::extract(FrontQueues& frontQueues) {
    return frontQueues.selectAndPop(0);
}

}

}