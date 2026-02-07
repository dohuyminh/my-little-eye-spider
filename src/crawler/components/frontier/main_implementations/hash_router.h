#pragma once

#include "../i_back_router.h"

namespace crawler {

namespace components {

class HashBackRouter : public IBackRouter {
public:

    HashBackRouter(std::size_t numBackQueues);

    std::size_t routeURL(const types::URL& url) override;

private:
    std::size_t numBackQueues_;
};

} // namespace components

} // namespace crawler