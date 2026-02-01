#pragma once

#include "../i_back_router.h"

namespace crawler {

namespace component {

class SampleRouter : public IBackRouter {
public:
    std::size_t routeURL(const URL& url) override;
};

}

}