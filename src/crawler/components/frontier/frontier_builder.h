#pragma once

#include <memory>
#include <stdexcept>
#include <utility>

#include "components/frontier/frontier_state.h"
#include "frontier.h"

namespace crawler {

namespace components {

template <typename FPType = void, typename FSType = void,
          typename BRType = void, typename BSType = void,
          typename BusType = NoBus>
class TemplateFrontierBuilder {
 public:
  TemplateFrontierBuilder() = default;

  TemplateFrontierBuilder(
      std::shared_ptr<FPType> frontPrioritizer,
      std::shared_ptr<FSType> frontSelector, std::shared_ptr<BRType> backRouter,
      std::shared_ptr<BSType> backSelector,
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> producingQueue,
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingQueue,
      std::unique_ptr<BusType> bus, std::size_t frontQueuesSize,
      std::size_t backQueuesSize, std::size_t batchSize)
      : frontPrioritizer_(frontPrioritizer),
        frontSelector_(frontSelector),
        backRouter_(backRouter),
        backSelector_(backSelector),
        producingQueue_(producingQueue),
        consumingQueue_(consumingQueue),
        bus_(std::move(bus)),
        frontQueuesSize_(frontQueuesSize),
        backQueuesSize_(backQueuesSize),
        batchSize_(batchSize) {}

  template <FrontPrioritizerType U>
  auto configurePrioritizer(std::shared_ptr<U> prioritizer) {
    return TemplateFrontierBuilder<U, FSType, BRType, BSType, BusType>(
        std::move(prioritizer), frontSelector_, backRouter_, backSelector_,
        producingQueue_, consumingQueue_, std::move(bus_), frontQueuesSize_,
        backQueuesSize_, batchSize_);
  }

  template <FrontPrioritizerType U, typename... Args>
  auto configurePrioritizer(Args&&... args)
    requires(std::is_constructible_v<U, Args...>)
  {
    return TemplateFrontierBuilder<U, FSType, BRType, BSType, BusType>(
        std::make_shared<U>(std::forward<Args>(args)...), frontSelector_,
        backRouter_, backSelector_, producingQueue_, consumingQueue_,
        std::move(bus_), frontQueuesSize_, backQueuesSize_, batchSize_);
  }

  template <FrontSelectorType U>
  auto configureFrontSelector(std::shared_ptr<U> selector) {
    return TemplateFrontierBuilder<FPType, U, BRType, BSType, BusType>(
        frontPrioritizer_, std::move(selector), backRouter_, backSelector_,
        producingQueue_, consumingQueue_, std::move(bus_), frontQueuesSize_,
        backQueuesSize_, batchSize_);
  }

  template <FrontSelectorType U, typename... Args>
  auto configureFrontSelector(Args&&... args)
    requires(std::is_constructible_v<U, Args...>)
  {
    return TemplateFrontierBuilder<FPType, U, BRType, BSType, BusType>(
        frontPrioritizer_, std::make_shared<U>(std::forward<Args>(args)...),
        backRouter_, backSelector_, producingQueue_, consumingQueue_,
        std::move(bus_), frontQueuesSize_, backQueuesSize_, batchSize_);
  }

  template <BackRouterType U>
  auto configureBackRouter(std::shared_ptr<U> router) {
    return TemplateFrontierBuilder<FPType, FSType, U, BSType, BusType>(
        frontPrioritizer_, frontSelector_, std::move(router), backSelector_,
        producingQueue_, consumingQueue_, std::move(bus_), frontQueuesSize_,
        backQueuesSize_, batchSize_);
  }

  template <BackRouterType U, typename... Args>
  auto configureBackRouter(Args&&... args)
    requires(std::is_constructible_v<U, Args...>)
  {
    return TemplateFrontierBuilder<FPType, FSType, U, BSType, BusType>(
        frontPrioritizer_, frontSelector_,
        std::make_shared<U>(std::forward<Args>(args)...), backSelector_,
        producingQueue_, consumingQueue_, std::move(bus_), frontQueuesSize_,
        backQueuesSize_, batchSize_);
  }

  template <BackSelectorType U>
  auto configureBackSelector(std::shared_ptr<U> selector) {
    return TemplateFrontierBuilder<FPType, FSType, BRType, U, BusType>(
        frontPrioritizer_, frontSelector_, backRouter_, std::move(selector),
        producingQueue_, consumingQueue_, std::move(bus_), frontQueuesSize_,
        backQueuesSize_, batchSize_);
  }

  template <BackSelectorType U, typename... Args>
  auto configureBackSelector(Args&&... args)
    requires(std::is_constructible_v<U, Args...>)
  {
    return TemplateFrontierBuilder<FPType, FSType, BRType, U, BusType>(
        frontPrioritizer_, frontSelector_, backRouter_,
        std::make_shared<U>(std::forward<Args>(args)...), producingQueue_,
        consumingQueue_, std::move(bus_), frontQueuesSize_, backQueuesSize_,
        batchSize_);
  }

  auto& configureProducingQueue(
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> queue) {
    producingQueue_ = std::move(queue);
    return *this;
  }

  auto& configureConsumingQueue(
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> queue) {
    consumingQueue_ = std::move(queue);
    return *this;
  }

  auto& configureNumFrontQueues(std::size_t numQueues) {
    frontQueuesSize_ = numQueues;
    return *this;
  }

  auto& configureNumBackQueues(std::size_t numQueues) {
    backQueuesSize_ = numQueues;
    return *this;
  }

  auto& configureProcessingBatchSize(std::size_t batchSize) {
    batchSize_ = batchSize;
    return *this;
  }

  template <typename U, typename... AdditionalArgs>
  auto configureUpdateBus(std::size_t updateBatchSize, AdditionalArgs&&... args)
    requires(ValidBusType<U, FPType, FSType, BRType, BSType> &&
             std::constructible_from<
                 U, std::shared_ptr<FPType>, std::shared_ptr<FSType>,
                 std::shared_ptr<BRType>, std::shared_ptr<BSType>, std::size_t,
                 AdditionalArgs && ...>)
  {
    return TemplateFrontierBuilder<FPType, FSType, BRType, BSType, U>(
        frontPrioritizer_, frontSelector_, backRouter_, backSelector_,
        producingQueue_, consumingQueue_,
        std::make_unique<U>(frontPrioritizer_, frontSelector_, backRouter_,
                            backSelector_, updateBatchSize,
                            std::forward<AdditionalArgs>(args)...),
        frontQueuesSize_, backQueuesSize_, batchSize_);
  }

  auto get()
    requires(!std::is_void_v<FPType> && !std::is_void_v<FSType> &&
             !std::is_void_v<BRType> && !std::is_void_v<BSType>)
  {
    if (frontQueuesSize_ == 0 || backQueuesSize_ == 0) {
      throw std::invalid_argument(
          "Template Frontier Builder: number of front/back queues must be a "
          "positive integer");
    }

    if (frontPrioritizer_ == nullptr) {
      throw std::invalid_argument(
          "Template Frontier Builder: missing prioritizer's configuration");
    }

    if (frontSelector_ == nullptr) {
      throw std::invalid_argument(
          "Template Frontier Builder: missing front selector's configuration");
    }

    if (backRouter_ == nullptr) {
      throw std::invalid_argument(
          "Template Frontier Builder: missing back router's configuration");
    }

    if (backSelector_ == nullptr) {
      throw std::invalid_argument(
          "Template Frontier Builder: missing back selector's configuration");
    }

    if (producingQueue_ == nullptr) {
      throw std::invalid_argument(
          "Template Frontier Builder: missing producing queue's configuration");
    }

    if (consumingQueue_ == nullptr) {
      throw std::invalid_argument(
          "Template Frontier Builder: missing consuming queue's configuration");
    }

    return Frontier(producingQueue_, consumingQueue_, frontQueuesSize_,
                    backQueuesSize_, frontPrioritizer_, frontSelector_,
                    backRouter_, backSelector_, std::move(bus_), batchSize_);
  }

 private:
  std::shared_ptr<FPType> frontPrioritizer_{nullptr};
  std::shared_ptr<FSType> frontSelector_{nullptr};
  std::shared_ptr<BRType> backRouter_{nullptr};
  std::shared_ptr<BSType> backSelector_{nullptr};

  std::unique_ptr<BusType> bus_{nullptr};

  std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> producingQueue_{
      nullptr};
  std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingQueue_{
      nullptr};

  std::size_t frontQueuesSize_{1};
  std::size_t backQueuesSize_{1};
  std::size_t batchSize_{0};
};

using FrontierBuilder = TemplateFrontierBuilder<>;

}  // namespace components

}  // namespace crawler
