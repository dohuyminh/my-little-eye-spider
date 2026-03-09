#include "sample_prioritizer.h"

namespace crawler {

namespace components {

std::pair<types::URL, std::size_t> SamplePrioritizer::selectQueue(
    const std::string& url) {
  return {types::URL(url), 0};
}

}  // namespace components

}  // namespace crawler