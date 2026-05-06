#pragma once

#include <memory>

#include "components/frontier/frontier_builder.h"
#include "frontier/default_back_router.h"
#include "frontier/default_front_prioritizer.h"
#include "frontier/default_front_selector.h"
#include "setup/frontier/default_back_selector.h"
#include "types/url.h"

namespace crawler::setup {

auto configureFrontier() {
  return components::FrontierBuilder{}
      .configureNumFrontQueues(20)
      .configureNumBackQueues(20)
      .configureProcessingBatchSize(20)
      .configureProducingQueue(
          std::make_shared<moodycamel::ConcurrentQueue<types::URL>>())
      .configureConsumingQueue(
          std::make_shared<moodycamel::ConcurrentQueue<types::URL>>())
      .configurePrioritizer<DefaultFrontPrioritizer>()
      .configureFrontSelector<DefaultFrontSelector>(4, 1)
      .configureBackRouter<DefaultBackRouter>(20)
      .configureBackSelector<DefaultBackSelector>()
      .get();
}

}  // namespace crawler::setup
