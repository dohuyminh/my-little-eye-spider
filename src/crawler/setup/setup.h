#pragma once

#include <memory>

#include "components/frontier/frontier_builder.h"
#include "components/worker/worker_pool.h"
#include "frontier/default_back_router.h"
#include "frontier/default_back_selector.h"
#include "frontier/default_front_prioritizer.h"
#include "frontier/default_front_selector.h"
#include "frontier/default_update_bus.h"
#include "setup/frontier/default_frontier_return_type.h"
#include "setup/worker/default_worker.h"

namespace crawler::setup {

auto configureFrontier() {
  return components::FrontierBuilder{}
      .configureFrontierReturnType<DefaultFrontierReturnType>()
      .configureNumFrontQueues(20)
      .configureNumBackQueues(20)
      .configureProcessingBatchSize(20)
      .configureProducingQueue(
          std::make_shared<
              moodycamel::ConcurrentQueue<DefaultFrontierReturnType>>())
      .configureConsumingQueue(
          std::make_shared<moodycamel::ConcurrentQueue<std::string>>())
      .configurePrioritizer<DefaultFrontPrioritizer>()
      .configureFrontSelector<DefaultFrontSelector>(4, 1)
      .configureBackRouter<DefaultBackRouter>(20)
      .configureBackSelector<DefaultBackSelector>()
      .configureUpdateBus<DefaultUpdateBus>(10)
      .get();
}

auto configureWorkerPool(
    decltype(configureFrontier())& frontierRef,
    std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingQueue) {
  using FrontierType = decltype(configureFrontier());
  return components::WorkerPool<FrontierType, DefaultWorker>(
      30, 100, DefaultWorker(producingQueue, frontierRef));
}

}  // namespace crawler::setup
