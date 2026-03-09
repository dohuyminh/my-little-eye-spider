#pragma once

#include <atomic>
#include <mutex>
#include <thread>

namespace crawler {

namespace types {

class Runnable {
 public:
  virtual void preLoop();

  void run();

  virtual void runImpl() = 0;

  bool isRunning() const noexcept { return isRunning_; }

  virtual void stop();

  ~Runnable() = default;

 private:
  std::thread eventLoopThread_;
  std::mutex runMutex_;
  bool isRunning_{false};
};

}  // namespace types

}  // namespace crawler