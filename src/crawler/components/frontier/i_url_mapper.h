#pragma once

#include "types/url.h"

namespace crawler {

namespace component {

class IURLMapper {
public:
    virtual double getPriority(const std::string& url) = 0;
    virtual ~IURLMapper() = default;
};

}

}