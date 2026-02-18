#include "worker.h"

#include <iostream>

#include "services/html_downloader.h"

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

  {
    std::unique_lock<std::mutex> printLock(printMutex_);
    // for now, just print the URL out
    std::cout << "[Consumer] Extracted URL: " << url_.url() << '\n';
    std::cout << "  Scheme: " << url_.scheme() << '\n';
    std::cout << "  Domain: " << url_.domain() << '\n';
    std::cout << "  Port: " << url_.port() << '\n';
    std::cout << "  Path: " << url_.path() << '\n';
    std::cout << "  Fragment: " << url_.fragment() << '\n';

    /**
     * Each worker performs the following operations:
     * 1. Download HTML
     * 2. Parse HTML
     * 3. Extract links/other resources
     * 4. Content seen/URL seen -> feed back to frontier
     */

    if (response.code() == 200) {
      std::cout << response.content() << '\n';
    } else {
      std::cout << "Cannot process URL\n";
    }
  }
}

}  // namespace components

}  // namespace crawler