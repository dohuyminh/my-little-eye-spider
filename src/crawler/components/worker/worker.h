#pragma once

#include <memory>

#include "moodycamel/concurrentqueue.h"
#include "types/url.h"

namespace crawler {

namespace components {

class Worker {
 public:
  Worker(
      std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs,
      types::URL&& url);

  void doWork();

 private:
  types::URL url_;

  std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs_;

  // for printing purposes; remove later
  static std::mutex printMutex_;
};

}  // namespace components

}  // namespace crawler