#pragma once

#include <atomic>
#include <thread>

namespace crawler {

namespace types {

class Runnable {
 public:
  virtual void preLoop();

  void run();

  virtual void runImpl() = 0;

  bool isRunning() const noexcept { return isRunning_.load(); }

  virtual void stop();

  virtual ~Runnable() = default;

 private:
  std::thread eventLoopThread_;
  std::atomic<bool> isRunning_{false};
};

class RAIIRunnable {
 public:
  RAIIRunnable(std::unique_ptr<Runnable> runnable);
  ~RAIIRunnable();

 private:
  std::unique_ptr<Runnable> runnable_;
};

}  // namespace types

}  // namespace crawler
