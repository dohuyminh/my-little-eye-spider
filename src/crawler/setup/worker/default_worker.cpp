#include "default_worker.h"

#include <string>

#include "rewards.h"
#include "services/html_downloader.h"
#include "services/html_parser.h"
#include "types/url.h"

namespace crawler::setup {

DefaultWorker::DefaultWorker(
    std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingQueue,
    DefaultFrontier& frontier)
    : components::Worker<DefaultFrontier>(producingQueue, frontier) {}

void DefaultWorker::doWork() {
  services::html::Downloader downloader;
  services::curl::Response response = downloader(data_.url());

  if (response.code() != 200) {
    return;
  }

  services::html::HTMLParser<services::html::LexborParser> parser;
  try {
    services::html::ParseResult result{parser.parse(response.content())};
    for (const auto& nextUrl : result.outlinks()) {
      // process outlinks and insert back to frontier
      // check if nextUrl is a relative URL
      types::URL baseURL{data_.url()};
      if (baseURL.applyPath(nextUrl)) {
        producingURLs_->try_enqueue(baseURL.to_string());
      } else {
        producingURLs_->try_enqueue(nextUrl);
      }
    }

    double r{reward(data_.url())};
    data_.setNormalizedReward(r);
    frontier_.sendUpdate(data_.updateCtx());

  } catch (std::invalid_argument const& e) {
    return;
  }
}

}  // namespace crawler::setup
