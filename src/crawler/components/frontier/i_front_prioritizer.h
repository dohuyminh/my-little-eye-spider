#pragma once

#include "types/url.h"
#include <utility>

namespace crawler {

namespace component {

class IFrontPrioritizer {
public:
    virtual std::pair<URL, std::size_t> selectQueue(const std::string& url) = 0;
    virtual ~IFrontPrioritizer() = default;
};

}

}