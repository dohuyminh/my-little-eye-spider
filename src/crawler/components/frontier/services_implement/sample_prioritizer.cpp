#include "sample_prioritizer.h"

namespace crawler {

namespace component {

std::pair<URL, std::size_t> SamplePrioritizer::selectQueue(const std::string& url) {
    return {URL(url, 0), 0};
}

}

}