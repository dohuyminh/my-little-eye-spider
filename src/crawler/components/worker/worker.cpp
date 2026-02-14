#include "worker.h"

#include <iostream>

namespace crawler {

namespace components {

Worker::Worker(
    std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs,
    types::URL&& url
) : 
    producingURLs_(producingURLs), 
    url_(std::move(url))
{}

void Worker::doWork() {
    // for now, just print the URL out
    std::cout << "[Consumer] Extracted URL: " << url_.url() << '\n';
    std::cout << "  Scheme: " << url_.scheme() << '\n';
    std::cout << "  Domain: " << url_.domain() << '\n';
    std::cout << "  Port: " << url_.port() << '\n';
    std::cout << "  Path: " << url_.path() << '\n';
    std::cout << "  Fragment: " << url_.fragment() << '\n';
}

}

}