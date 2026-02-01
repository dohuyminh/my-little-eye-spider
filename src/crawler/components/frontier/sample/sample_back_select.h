#pragma once

#include "../i_back_selector.h"

namespace crawler {

namespace components {

class SampleBackSelector : public IBackSelector {
public:
    std::optional<types::URL> extract(BackQueues& backQueues) override;
};

}

}