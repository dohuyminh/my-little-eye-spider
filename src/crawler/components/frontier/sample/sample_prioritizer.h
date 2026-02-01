#pragma once

#include "../i_front_prioritizer.h"

namespace crawler {

namespace components {

class SamplePrioritizer : public IFrontPrioritizer {
public:
    std::pair<types::URL, std::size_t> selectQueue(const std::string& url) override;
};

}

}