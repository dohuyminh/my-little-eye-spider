#pragma once

#include "../i_back_router.h"
#include <mutex>

namespace crawler {

namespace component {

class RoundRobinBackRouter : public IBackRouter {

private:
    std::size_t pointer_{ 0 };

    std::mutex mutex_;
};

}

}