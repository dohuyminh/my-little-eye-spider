#pragma once

#include "types/url.h"

namespace crawler {

namespace component {

class IBackRouter {
public:
    virtual std::size_t routeURL(const URL& url) = 0;
    virtual ~IBackRouter() = default;
};

}

}