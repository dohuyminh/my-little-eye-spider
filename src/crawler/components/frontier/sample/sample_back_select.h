#pragma once

#include "../i_back_selector.h"

namespace crawler {

namespace component {

class SampleBackSelector : public IBackSelector {
public:
    std::optional<URL> extract(BackQueues& backQueues) override;
};

}

}