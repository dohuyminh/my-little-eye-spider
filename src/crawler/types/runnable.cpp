#include "runnable.h"

#include <stdexcept>

namespace crawler {

namespace types {

void Runnable::preLoop() {}

void Runnable::run() {
  if (eventLoopThread_.joinable()) {
    return;
  }

  // only start if not already running
  bool expected{false};
  if (!isRunning_.compare_exchange_strong(expected, true)) {
    return;
  }

  try {
    preLoop();

    // Create thread to run implementation
    eventLoopThread_ = std::thread([this]() {
      while (isRunning_.load()) {
        // Call the actual implementation
        runImpl();
      }
    });
  } catch (...) {
    isRunning_.store(false);
    throw std::runtime_error("Runnable: failed to launch background thread");
  }
}

void Runnable::stop() {
  isRunning_.store(false);

  if (eventLoopThread_.joinable()) {
    eventLoopThread_.join();
  }
}

RAIIRunnable::RAIIRunnable(std::unique_ptr<Runnable> runnable)
    : runnable_(std::move(runnable)) {
  if (runnable_ == nullptr) {
    throw std::invalid_argument("Runnable cannot be null");
  }

  runnable_->run();
}

RAIIRunnable::~RAIIRunnable() { runnable_->stop(); }

}  // namespace types

}  // namespace crawler
