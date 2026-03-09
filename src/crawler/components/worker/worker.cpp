#include "worker.h"

#include <iostream>

#include "services/html_downloader.h"
#include "services/html_parser.h"

namespace crawler {

namespace components {

std::mutex Worker::printMutex_;

Worker::Worker(
    std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs,
    types::URL&& url)
    : producingURLs_(producingURLs), url_(std::move(url)) {}

void Worker::doWork() {
  services::html::Downloader downloader;
  services::curl::Response response = downloader(url_);

  if (response.code() != 200) {
    return;
  }

  services::html::HTMLParser<services::html::LexborParser> parser;
  try {
    services::html::ParseResult result{parser.parse(response.content())};
    for (const auto& nextUrl : result.outlinks()) {
      // process outlinks and insert back to frontier
    }

  } catch (std::invalid_argument const& e) {
    return;
  }
}

}  // namespace components

}  // namespace crawler