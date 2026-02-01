#pragma once

#include "types/url.h"

namespace crawler {

namespace components {

class IBackRouter {
public:
    virtual std::size_t routeURL(const types::URL& url) = 0;
    virtual ~IBackRouter() = default;
};

}

}