#pragma once

#include "components/frontier/i_front_selector.h"
#include "frontier.h"

namespace crawler {

namespace component {

class FrontierBuilder {
public:
    
    FrontierBuilder() = default;

    void setFrontQueuesSize(std::size_t frontQueueSize);

    void setBackQueuesSize(std::size_t backQueueSize);

    void setPrioritizer(IFrontPrioritizer* prioritizer);

    void setFrontSelector(IFrontSelector* selector);

    void setRouter(IBackRouter* router);

    void setBackSelector(IBackSelector* selector);

    bool canBuild() const noexcept {
        return frontQueuesSize_ > 0 && 
                backQueuesSize_ > 0 && 
                prioritizer_ && 
                frontSelector_ && 
                router_ && 
                backSelector_;
    }

    void reset() noexcept {
        frontQueuesSize_ = backQueuesSize_ = 0;
        delete prioritizer_;
        prioritizer_ = nullptr;
        delete frontSelector_;
        frontSelector_ = nullptr;
        delete router_;
        router_ = nullptr;
        delete backSelector_;
        backSelector_ = nullptr;
    }

    Frontier get();

    ~FrontierBuilder() {
        reset();
    }

private:
    std::size_t frontQueuesSize_{ 0 };
    std::size_t backQueuesSize_{ 0 };
    IFrontPrioritizer* prioritizer_{ nullptr };
    IFrontSelector* frontSelector_{ nullptr };
    IBackRouter* router_{ nullptr };
    IBackSelector* backSelector_{ nullptr };
};

}

}