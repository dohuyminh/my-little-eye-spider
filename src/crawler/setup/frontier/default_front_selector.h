#pragma once

#include <random>

#include "components/frontier/multiqueue_containers.h"
#include "types/url.h"

namespace crawler {

namespace setup {

class DefaultFrontSelector {
 public:
  using FrontQueueContainer = components::MultiQueueContainers<
      components::MoodyCamelConcurrentQueueWrapper<types::URL>>;

  using StateUpdatePacket = std::tuple<std::size_t, double>;

  DefaultFrontSelector(std::size_t numArms = 4, double gamma = 0.1);

  std::optional<types::URL> extract(FrontQueueContainer& frontQueues);
  std::vector<types::URL> extractBatch(FrontQueueContainer& frontQueues,
                                       std::size_t maxCount);

  void update(std::size_t arm, double reward);

 private:
  std::size_t numArms_;
  std::size_t lastArm_{0};
  double gamma_;

  std::vector<double> weights_;
  std::vector<double> armProbabilities_;

  std::mt19937 gen_;

  void updatePolicy();
};

}  // namespace setup

}  // namespace crawler
