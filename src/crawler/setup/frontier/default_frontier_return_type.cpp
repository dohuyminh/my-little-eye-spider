#include "default_frontier_return_type.h"

#include "types/url.h"

namespace crawler::setup {

DefaultFrontierReturnType::DefaultFrontierReturnType(const types::URL& url,
                                                     std::size_t arm,
                                                     double reward)
    : url_(url), arm_(arm), reward_(reward) {}

const types::URL& DefaultFrontierReturnType::url() const noexcept {
  return url_;
}

void DefaultFrontierReturnType::setArm(std::size_t arm) noexcept { arm_ = arm; }

void DefaultFrontierReturnType::setNormalizedReward(double rawReward) noexcept {
  reward_ = 1.0 / (1.0 + std::exp(-rawReward));
}

std::tuple<std::tuple<>, std::tuple<std::size_t, double>, std::tuple<>,
           std::tuple<>>
DefaultFrontierReturnType::updateCtx() const noexcept {
  return {{}, {arm_, reward_}, {}, {}};
}

}  // namespace crawler::setup
