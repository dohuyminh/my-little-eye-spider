#pragma once

#include "../i_front_prioritizer.h"
#include <mutex>

namespace crawler {

namespace components {

class RoundRobinPrioritizer : public IFrontPrioritizer {
public:
    RoundRobinPrioritizer(std::size_t numQueues);

    std::pair<types::URL, std::size_t> selectQueue(const std::string& url) override;
    
private:
    std::size_t numQueues_;
    std::size_t pointer_{ 0 };

    std::mutex mutex_;
};

}
    
}