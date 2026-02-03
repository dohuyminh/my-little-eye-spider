#include "sample_back_select.h"

namespace crawler {

namespace components {

std::optional<types::URL> SampleBackSelector::extract(BackQueues& backQueues) {
    return backQueues.selectAndPop(0);
}

}

}