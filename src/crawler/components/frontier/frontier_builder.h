#pragma once

#include "frontier.h"

namespace crawler {

namespace components {

class FrontierBuilder {
public:
    
    FrontierBuilder() = default;

    void setFrontQueuesSize(std::size_t frontQueueSize);

    void setBackQueuesSize(std::size_t backQueueSize);

    void setBatchSize(std::size_t batchSize);

    void setProducingQueue(std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> producingQueue);

    void setConsumingQueue(std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingQueue);

    void setPrioritizer(IFrontPrioritizer* prioritizer);

    void setFrontSelector(IFrontSelector* selector);

    void setRouter(IBackRouter* router);

    void setBackSelector(IBackSelector* selector);

    bool canBuild() const noexcept {
        return frontQueuesSize_ > 0 && 
                backQueuesSize_ > 0 && 
                producingQueue_ != nullptr &&
                consumingQueue_ != nullptr &&
                prioritizer_ && 
                frontSelector_ && 
                router_ && 
                backSelector_;
    }

    void reset() noexcept {
        frontQueuesSize_ = backQueuesSize_ = 0;
        batchSize_ = 1;
        producingQueue_ = nullptr;
        consumingQueue_ = nullptr;
        delete prioritizer_;
        prioritizer_ = nullptr;
        delete frontSelector_;
        frontSelector_ = nullptr;
        delete router_;
        router_ = nullptr;
        delete backSelector_;
        backSelector_ = nullptr;
    }

    std::unique_ptr<Frontier> get();

    ~FrontierBuilder() {
        reset();
    }

private:
    std::size_t frontQueuesSize_{ 0 };
    std::size_t backQueuesSize_{ 0 };
    std::size_t batchSize_{ 1 };
    std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> producingQueue_{ nullptr };
    std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingQueue_{ nullptr };
    IFrontPrioritizer* prioritizer_{ nullptr };
    IFrontSelector* frontSelector_{ nullptr };
    IBackRouter* router_{ nullptr };
    IBackSelector* backSelector_{ nullptr };
};

}

}