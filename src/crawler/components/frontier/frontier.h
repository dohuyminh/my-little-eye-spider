#pragma once

#include <concepts>
#include <memory>
#include <type_traits>
#include <vector>

#include "components/frontier/frontier_state.h"
#include "i_back_router.h"
#include "i_back_selector.h"
#include "i_front_prioritizer.h"
#include "i_front_selector.h"
#include "multiqueue_containers.h"
#include "types/runnable.h"

namespace crawler {

namespace components {

template <FrontPrioritizerType FrontPrioritizer,
          FrontSelectorType FrontSelector, BackRouterType BackRouter,
          BackSelectorType BackSelector>
struct DataType {
  using type = std::conditional<
      StatefulFrontierComponent<FrontPrioritizer> ||
          StatefulFrontierComponent<FrontSelector> ||
          StatefulFrontierComponent<BackRouter> ||
          StatefulFrontierComponent<BackSelector>,
      typename UpdateQueueBus<FrontPrioritizer, FrontSelector, BackRouter,
                              BackSelector>::UpdatePacketType,
      void>;
};

template <FrontPrioritizerType FrontPrioritizer,
          FrontSelectorType FrontSelector, BackRouterType BackRouter,
          BackSelectorType BackSelector, typename UpdateBusType = NoBus>
  requires(!IsStatefulConfig<FrontPrioritizer, FrontSelector, BackRouter,
                             BackSelector> ||
           (!std::same_as<UpdateBusType, NoBus> &&
            ValidBusType<UpdateBusType, FrontPrioritizer, FrontSelector,
                         BackRouter, BackSelector>))
class Frontier : public types::Runnable {
 public:
  Frontier(
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> producingQueue,
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingQueue,
      std::size_t numFrontQueues, std::size_t numBackQueues,
      std::shared_ptr<FrontPrioritizer> frontPrioritizer,
      std::shared_ptr<FrontSelector> frontSelector,
      std::shared_ptr<BackRouter> backRouter,
      std::shared_ptr<BackSelector> backSelector,
      std::unique_ptr<UpdateBusType> updateBus = nullptr,
      std::size_t batchSize = 1)
      : producingQueue_(producingQueue),
        consumingQueue_(consumingQueue),
        frontQueues_(numFrontQueues),
        backQueues_(numBackQueues),
        prioritizer_(frontPrioritizer),
        frontSelector_(frontSelector),
        router_(backRouter),
        backSelector_(backSelector),
        batchSize_(batchSize > 0 ? batchSize : 1) {
    if constexpr (isStateful()) {
      updateBus_ = std::move(updateBus);
      updateBus_->run();
    }
  }

  static constexpr bool isStateful() {
    return StatefulFrontierComponent<FrontPrioritizer> ||
           StatefulFrontierComponent<FrontSelector> ||
           StatefulFrontierComponent<BackRouter> ||
           StatefulFrontierComponent<BackSelector>;
  }

  void sendUpdate(
      const typename UpdateQueueBus<FrontPrioritizer, FrontSelector, BackRouter,
                                    BackSelector>::UpdatePacketType& pkt)
    requires(StatefulFrontierComponent<FrontPrioritizer> ||
             StatefulFrontierComponent<FrontSelector> ||
             StatefulFrontierComponent<BackRouter> ||
             StatefulFrontierComponent<BackSelector>)
  {
    updateBus_->insertUpdatePacket(pkt);
  }

  void insertToFrontQueue(const std::string& url) {
    try {
      auto [urlObj, queueIndex] = prioritizer_->selectQueue(url);
      for (std::size_t i : queueIndex) {
        frontQueues_.enqueue(i, std::move(urlObj));
      }
    } catch (const std::invalid_argument& e) {
      // Ignore invalid URL
    }
  }

  std::vector<types::URL> popFrontBatch(std::size_t maxCount) {
    return frontSelector_->extractBatch(frontQueues_, maxCount);
  }

  void insertToBackQueue(const std::vector<types::URL>& urls) {
    for (const types::URL& url : urls) {
      std::size_t backQueueIndex = router_->routeURL(url);
      backQueues_.enqueue(backQueueIndex, url);
    }
  }

  std::vector<types::URL> popBackBatch(std::size_t maxCount) {
    return backSelector_->extractBatch(backQueues_, maxCount);
  }

  void runImpl() override {
    // Process front queues → back queues (single batch extract, up to
    // batchSize_)
    auto frontBatch = popFrontBatch(batchSize_);
    if (!frontBatch.empty()) {
      insertToBackQueue(frontBatch);
    }

    // Process back queues → shared queue for workers (single batch extract, up
    // to batchSize_)
    auto backBatch = popBackBatch(batchSize_);
    producingQueue_->enqueue_bulk(backBatch.begin(), backBatch.size());
  }

  ~Frontier() {
    if (updateBus_ != nullptr) {
      updateBus_->stop();
    }
  }

 private:
  using FrontQueueContainer =
      MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<types::URL>>;
  using BackQueueContainer = MultiQueueContainers<StdQueueWrapper<types::URL>>;

  // front and back queues
  FrontQueueContainer frontQueues_;
  BackQueueContainer backQueues_;

  // for assigning each URL a respective priority to be put into front queue
  std::shared_ptr<FrontPrioritizer> prioritizer_;

  // for selecting URL for back queue router
  std::shared_ptr<FrontSelector> frontSelector_;

  // for assigning URLs into back queue
  std::shared_ptr<BackRouter> router_;

  // for rate-limiting and URL extraction (results to be handed to)
  std::shared_ptr<BackSelector> backSelector_;

  // Frontier is the producer of back queue URLs, to be consumed by worker
  // threads
  std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> producingQueue_;

  // Frontier also consumes incoming URLs from the worker threads via front
  // queues
  std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingQueue_;

  // Frontier's components receive updates via the update bus
  // If the frontier is stateless, this object will not exist, and no background
  // thread is launched
  std::unique_ptr<UpdateBusType> updateBus_{};

  std::size_t batchSize_;
};

}  // namespace components

}  // namespace crawler
