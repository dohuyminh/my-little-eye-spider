#pragma once

#include "../i_back_router.h"

namespace crawler {

namespace components {

class SampleRouter : public IBackRouter {
public:
    std::size_t routeURL(const types::URL& url) override;
};

}

}