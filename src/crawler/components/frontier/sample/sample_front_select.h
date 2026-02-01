#pragma once

#include "../i_front_selector.h"

namespace crawler {

namespace components {

class SampleFrontSelector : public IFrontSelector {
public:
    std::optional<types::URL> extract(FrontQueues& frontQueues) override;
};

}

}