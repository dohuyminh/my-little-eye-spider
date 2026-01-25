#pragma once

#include "../i_front_selector.h"

namespace crawler {

namespace component {

class SampleFrontSelector : public IFrontSelector {
public:
    std::optional<URL> extract(FrontQueues& frontQueues) override;
};

}

}