#include "sample_front_select.h"

namespace crawler {

namespace component {

std::optional<URL> SampleFrontSelector::extract(FrontQueues& frontQueues) {
    return frontQueues.selectAndPop(0);
}

}

}