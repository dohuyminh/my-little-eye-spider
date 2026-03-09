#include "runnable.h"

namespace crawler {

namespace types {

void Runnable::preLoop() {}

void Runnable::run() {
  // only start if not already running
  if (isRunning_) {
    return;
  }

  {
    std::unique_lock<std::mutex> lock(runMutex_);
    isRunning_ = true;
  }

  preLoop();

  // Create thread to run implementation
  eventLoopThread_ = std::thread([this]() {
    while (true) {
      // Call the actual implementation
      runImpl();

      {
        // Lock to check running state
        std::unique_lock<std::mutex> lock(runMutex_);
        if (!isRunning_) {
          break;
        }
      }
    }
  });
}

void Runnable::stop() {
  {
    std::unique_lock<std::mutex> lock(runMutex_);
    if (!isRunning_) {
      return;
    }
    isRunning_ = false;
  }

  if (eventLoopThread_.joinable()) {
    eventLoopThread_.join();
  }
}

}  // namespace types

}  // namespace crawler