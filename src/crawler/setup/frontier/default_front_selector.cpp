#include "default_front_selector.h"

namespace crawler::setup {

DefaultFrontSelector::DefaultFrontSelector(std::size_t numArms, double gamma)
    : numArms_(numArms),
      gamma_(gamma),
      weights_(numArms_, 1.0),
      armProbabilities_(numArms_, 1.0 / numArms_),
      gen_(std::random_device{}()) {
  if (numArms_ == 0) {
    throw std::invalid_argument("Number of arms must be positive");
  }
}

std::optional<DefaultFrontierReturnType> DefaultFrontSelector::extract(
    FrontQueueContainer& frontQueues) {
  if (frontQueues.numQueues() != numArms_) [[unlikely]] {
    throw std::invalid_argument(
        "Number of queues does not match number of arms");
  }

  updatePolicy();

  std::discrete_distribution<> distribution(armProbabilities_.begin(),
                                            armProbabilities_.end());

  lastArm_ = distribution(gen_);
  std::optional<DefaultFrontierReturnType> url{frontQueues.dequeue(lastArm_)};

  if (url.has_value()) {
    url->setArm(lastArm_);
  }

  return url;
}

std::vector<DefaultFrontierReturnType> DefaultFrontSelector::extractBatch(
    FrontQueueContainer& frontQueues, std::size_t batchSize) {
  std::vector<DefaultFrontierReturnType> urls;
  for (std::size_t i = 0; i < batchSize; ++i) {
    auto url = extract(frontQueues);
    if (url.has_value()) {
      urls.push_back(url.value());
    }
  }
  return urls;
}

void DefaultFrontSelector::update(std::size_t arm, double reward) {
  if (arm >= numArms_) {
    throw std::invalid_argument("Invalid arm index");
  }

  if (reward < 0 || reward > 1) {
    throw std::invalid_argument("Reward must be between 0 and 1");
  }

  double p{armProbabilities_[arm]};
  if (p <= 0) {
    return;
  }

  // importance-weighted reward
  double estimatedReward{reward / p};

  // Exp3 weight update
  weights_[arm] *= std::exp(gamma_ * estimatedReward / numArms_);
}

void DefaultFrontSelector::updatePolicy() {
  double totalWeight{std::accumulate(weights_.begin(), weights_.end(), 0.0)};

  for (std::size_t i{0}; i < numArms_; ++i) {
    armProbabilities_[i] =
        (1 - gamma_) * weights_[i] / totalWeight + gamma_ / numArms_;
  }
}

}  // namespace crawler::setup
