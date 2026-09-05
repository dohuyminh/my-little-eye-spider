#include "default_worker.h"

#include <memory>
#include <string>

#include "rewards.h"
#include "services/http_get_handler.h"
#include "services/html_parser.h"
#include "setup/worker/url_insert.h"
#include "types/url.h"

namespace crawler::setup {

DefaultWorker::DefaultWorker(
    std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingQueue,
    DefaultFrontier& frontier)
    : components::Worker<DefaultFrontier>(producingQueue, frontier) {}

database::PostgreSQLWorkQueue workQueue_{database::getConnectionString(), 10};

void DefaultWorker::doWork() {
  services::html::HTTPGETHandler downloader;
  services::curl::Response response = downloader(data_.url());

  if (response.code() != 200) {
    return;
  }

  services::html::HTMLParser<services::html::LexborParser> parser;
  try {
    services::html::ParseResult result{parser.parse(response.content())};
    for (auto& nextUrl : result.outlinks()) {
      // process outlinks and insert back to frontier
      // check if nextUrl is a relative URL
      types::URL baseURL{data_.url()};
      if (baseURL.applyPath(nextUrl)) {
        nextUrl = baseURL.to_string();
        producingURLs_->try_enqueue(baseURL.to_string());
      } else {
        producingURLs_->try_enqueue(nextUrl);
      }
          
    }

    // insert current URL and its associated outlinks 
    auto status = DefaultWorker::inserter_.insertURL(data_.url().to_string(), result.outlinks());
    if (status != URLInserter::InsertStatus::INSERT_SUCCESS) {
      // log error; the error will influence the reward 
    }

    double r{reward(data_.url())};
    data_.setNormalizedReward(r);
    frontier_.sendUpdate(data_.updateCtx());

  } catch (std::invalid_argument const& e) {
    return;
  }
}

}  // namespace crawler::setup
