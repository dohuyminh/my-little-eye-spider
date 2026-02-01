#include "sample_back_select.h"

namespace crawler {

namespace component {

std::optional<URL> SampleBackSelector::extract(BackQueues& backQueues) {
    return backQueues.selectAndPop(0);
}

}

}