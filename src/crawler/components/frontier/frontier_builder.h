#pragma once

#include <stdexcept>

#include "frontier.h"

namespace crawler {

namespace components {

template <typename FPType = void, typename FSType = void,
          typename BRType = void, typename BSType = void>
class TemplateFrontierBuilder {
 public:
  TemplateFrontierBuilder() = default;

  TemplateFrontierBuilder(
      std::shared_ptr<FPType> frontPrioritizer,
      std::shared_ptr<FSType> frontSelector, std::shared_ptr<BRType> backRouter,
      std::shared_ptr<BSType> backSelector,
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> producingQueue,
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingQueue,
      std::size_t frontQueuesSize, std::size_t backQueuesSize,
      std::size_t batchSize)
      : frontPrioritizer_(frontPrioritizer),
        frontSelector_(frontSelector),
        backRouter_(backRouter),
        backSelector_(backSelector),
        producingQueue_(producingQueue),
        consumingQueue_(consumingQueue),
        frontQueuesSize_(frontQueuesSize),
        backQueuesSize_(backQueuesSize),
        batchSize_(batchSize) {}

  template <FrontPrioritizerType U>
  auto configurePrioritizer(std::shared_ptr<U> prioritizer) const {
    return TemplateFrontierBuilder<U, FSType, BRType, BSType>(
        std::move(prioritizer), frontSelector_, backRouter_, backSelector_,
        producingQueue_, consumingQueue_, frontQueuesSize_, backQueuesSize_,
        batchSize_);
  }

  template <FrontPrioritizerType U, typename... Args>
  auto configurePrioritizer(Args&&... args) const
    requires(std::is_constructible_v<U, Args...>)
  {
    return TemplateFrontierBuilder<U, FSType, BRType, BSType>(
        std::make_shared<U>(std::forward<Args>(args)...), frontSelector_,
        backRouter_, backSelector_, producingQueue_, consumingQueue_,
        frontQueuesSize_, backQueuesSize_, batchSize_);
  }

  template <FrontSelectorType U>
  auto configureFrontSelector(std::shared_ptr<U> selector) const {
    return TemplateFrontierBuilder<FPType, U, BRType, BSType>(
        frontPrioritizer_, std::move(selector), backRouter_, backSelector_,
        producingQueue_, consumingQueue_, frontQueuesSize_, backQueuesSize_,
        batchSize_);
  }

  template <FrontSelectorType U, typename... Args>
  auto configureFrontSelector(Args&&... args) const
    requires(std::is_constructible_v<U, Args...>)
  {
    return TemplateFrontierBuilder<FPType, U, BRType, BSType>(
        frontPrioritizer_, std::make_shared<U>(std::forward<Args>(args)...),
        backRouter_, backSelector_, producingQueue_, consumingQueue_,
        frontQueuesSize_, backQueuesSize_, batchSize_);
  }

  template <BackRouterType U>
  auto configureBackRouter(std::shared_ptr<U> router) const {
    return TemplateFrontierBuilder<FPType, FSType, U, BSType>(
        frontPrioritizer_, frontSelector_, std::move(router), backSelector_,
        producingQueue_, consumingQueue_, frontQueuesSize_, backQueuesSize_,
        batchSize_);
  }

  template <BackRouterType U, typename... Args>
  auto configureBackRouter(Args&&... args) const
    requires(std::is_constructible_v<U, Args...>)
  {
    return TemplateFrontierBuilder<FPType, FSType, U, BSType>(
        frontPrioritizer_, frontSelector_,
        std::make_shared<U>(std::forward<Args>(args)...), backSelector_,
        producingQueue_, consumingQueue_, frontQueuesSize_, backQueuesSize_,
        batchSize_);
  }

  template <BackSelectorType U>
  auto configureBackSelector(std::shared_ptr<U> selector) const {
    return TemplateFrontierBuilder<FPType, FSType, BRType, U>(
        frontPrioritizer_, frontSelector_, backRouter_, std::move(selector),
        producingQueue_, consumingQueue_, frontQueuesSize_, backQueuesSize_,
        batchSize_);
  }

  template <BackSelectorType U, typename... Args>
  auto configureBackSelector(Args&&... args) const
    requires(std::is_constructible_v<U, Args...>)
  {
    return TemplateFrontierBuilder<FPType, FSType, BRType, U>(
        frontPrioritizer_, frontSelector_, backRouter_,
        std::make_shared<U>(std::forward<Args>(args)...), producingQueue_,
        consumingQueue_, frontQueuesSize_, backQueuesSize_, batchSize_);
  }

  auto configureProducingQueue(
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> queue) const {
    auto copy = *this;
    copy.producingQueue_ = std::move(queue);
    return copy;
  }

  auto configureConsumingQueue(
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> queue) const {
    auto copy = *this;
    copy.consumingQueue_ = std::move(queue);
    return copy;
  }

  auto configureNumFrontQueues(std::size_t numQueues) const {
    auto copy = *this;
    copy.frontQueuesSize_ = numQueues;
    return copy;
  }

  auto configureNumBackQueues(std::size_t numQueues) const {
    auto copy = *this;
    copy.backQueuesSize_ = numQueues;
    return copy;
  }

  auto configureProcessingBatchSize(std::size_t batchSize) const {
    auto copy = *this;
    copy.batchSize_ = batchSize;
    return copy;
  }

  auto get() const
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
                    backRouter_, backSelector_, batchSize_);
  }

 private:
  std::shared_ptr<FPType> frontPrioritizer_{nullptr};
  std::shared_ptr<FSType> frontSelector_{nullptr};
  std::shared_ptr<BRType> backRouter_{nullptr};
  std::shared_ptr<BSType> backSelector_{nullptr};

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
