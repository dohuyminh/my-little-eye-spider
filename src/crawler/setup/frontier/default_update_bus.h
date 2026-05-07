#pragma once

#include <atomic>

#include "components/frontier/frontier_state.h"
#include "default_back_router.h"
#include "default_back_selector.h"
#include "default_front_prioritizer.h"
#include "default_front_selector.h"

namespace crawler::setup {

class DefaultUpdateBus
    : public components::UpdateQueueBus<DefaultFrontPrioritizer,
                                        DefaultFrontSelector, DefaultBackRouter,
                                        DefaultBackSelector> {
 public:
  DefaultUpdateBus(std::shared_ptr<DefaultFrontPrioritizer> frontPrioritizer,
                   std::shared_ptr<DefaultFrontSelector> frontSelector,
                   std::shared_ptr<DefaultBackRouter> backRouter,
                   std::shared_ptr<DefaultBackSelector> backSelector,
                   std::size_t updateBatchSize);

  bool canUpdate() override {
    return packetApproxCount_.load() >= updateBatchSize();
  }

  void modifyUpdateStatus() override {
    packetApproxCount_.fetch_sub(updateBatchSize());
  }

  void insertUpdatePacket(const UpdatePacketType& pkt) override {
    components::UpdateQueueBus<DefaultFrontPrioritizer, DefaultFrontSelector,
                               DefaultBackRouter,
                               DefaultBackSelector>::insertUpdatePacket(pkt);
    ++packetApproxCount_;
  }

 private:
  std::atomic<std::size_t> packetApproxCount_{0};
};

}  // namespace crawler::setup
