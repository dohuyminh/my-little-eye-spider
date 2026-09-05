#pragma once

#include "../frontier/default_back_router.h"
#include "../frontier/default_back_selector.h"
#include "../frontier/default_front_prioritizer.h"
#include "../frontier/default_front_selector.h"
#include "../frontier/default_update_bus.h"
#include "components/worker/worker.h"
#include "setup/worker/url_insert.h"

namespace crawler::setup {

using DefaultFrontier =
    components::Frontier<DefaultFrontierReturnType, DefaultFrontPrioritizer,
                         DefaultFrontSelector, DefaultBackRouter,
                         DefaultBackSelector, DefaultUpdateBus>;

class DefaultWorker : public components::Worker<DefaultFrontier> {
 public:
  DefaultWorker(
      std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs,
      DefaultFrontier& frontier);

  void doWork() override;

 private:
  static URLInserter inserter_; 
};

}  // namespace crawler::setup
