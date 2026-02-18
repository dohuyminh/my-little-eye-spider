#pragma once

#include <memory>
#include <vector>

#include "back_queues.h"
#include "front_queues.h"
#include "i_back_router.h"
#include "i_back_selector.h"
#include "i_front_prioritizer.h"
#include "i_front_selector.h"
#include "types/runnable.h"

namespace crawler {

namespace components {

class Frontier : public types::Runnable {
 public:
  Frontier(
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> producingQueue,
      std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingQueue,
      std::size_t numFrontQueues, std::size_t numBackQueues,
      IFrontPrioritizer* prioritizer, IFrontSelector* frontSelector,
      IBackRouter* router, IBackSelector* backSelector,
      std::size_t batchSize = 1);

  void insertToFrontQueue(const std::string& url);

  std::optional<types::URL> popFront();
  std::vector<types::URL> popFrontBatch(std::size_t maxCount);

  void insertToBackQueue(const std::vector<types::URL>& urls);

  void insertToBackQueue(std::vector<types::URL>&& urls);
  void insertToBackQueue(types::URL&& url);

  std::optional<types::URL> popBack();
  std::vector<types::URL> popBackBatch(std::size_t maxCount);

  void runImpl() override;

 private:
  // queues access
  FrontQueues frontQueues_;
  BackQueues backQueues_;

  // for assigning each URL a respective priority to be put into front queue
  std::unique_ptr<IFrontPrioritizer> prioritizer_;

  // for selecting URL for back queue router
  std::unique_ptr<IFrontSelector> frontSelector_;

  // for assigning URLs into back queue
  std::unique_ptr<IBackRouter> router_;

  // for rate-limiting and URL extraction (results to be handed to)
  std::unique_ptr<IBackSelector> backSelector_;

  // Frontier is the producer of back queue URLs, to be consumed by worker
  // threads
  std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> producingQueue_;

  // Frontier also consumes incoming URLs from the worker threads via front
  // queues
  std::shared_ptr<moodycamel::ConcurrentQueue<types::URL>> consumingQueue_;

  std::size_t batchSize_;
};

}  // namespace components

}  // namespace crawler