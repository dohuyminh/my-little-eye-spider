#include "default_worker.h"
#include <string>

#include "services/html_downloader.h"
#include "services/html_parser.h"

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
    }

  } catch (std::invalid_argument const& e) {
    return;
  }
}

}  // namespace crawler::setup
