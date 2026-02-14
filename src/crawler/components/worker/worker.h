#pragma once

#include "types/url.h"

#include "moodycamel/concurrentqueue.h"

#include <memory>

namespace crawler {

namespace components {

class Worker {
public:

    Worker(
        std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs, 
        types::URL&& url
    );

    void doWork();

private:

    types::URL url_;

    std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs_;

};

}

}