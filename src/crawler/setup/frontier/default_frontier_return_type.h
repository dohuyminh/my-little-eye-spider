#pragma once

#include <cmath>
#include <tuple>

#include "types/url.h"

namespace crawler::setup {

class DefaultFrontierReturnType {
 public:
  DefaultFrontierReturnType() = default;

  DefaultFrontierReturnType(const types::URL& url, std::size_t arm = 0,
                            double reward = -1);

  DefaultFrontierReturnType(const DefaultFrontierReturnType& other) noexcept =
      default;
  DefaultFrontierReturnType(DefaultFrontierReturnType&& other) noexcept =
      default;

  DefaultFrontierReturnType& operator=(
      const DefaultFrontierReturnType& other) noexcept = default;
  DefaultFrontierReturnType& operator=(
      DefaultFrontierReturnType&& other) noexcept = default;

  const types::URL& url() const noexcept;

  void setArm(std::size_t arm) noexcept;

  void setNormalizedReward(double rawReward) noexcept;

  std::tuple<std::tuple<>, std::tuple<std::size_t, double>, std::tuple<>,
             std::tuple<>>
  updateCtx() const noexcept;

 private:
  types::URL url_;

  std::size_t arm_;
  double reward_;
};

}  // namespace crawler::setup
