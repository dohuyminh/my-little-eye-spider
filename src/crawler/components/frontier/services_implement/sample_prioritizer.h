#pragma once

#include "../i_front_prioritizer.h"

namespace crawler {

namespace component {

class SamplePrioritizer : public IFrontPrioritizer {
public:
    std::pair<URL, std::size_t> selectQueue(const std::string& url) override;
};

}

}