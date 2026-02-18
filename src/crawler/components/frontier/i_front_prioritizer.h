#pragma once

#include <utility>

#include "types/url.h"

namespace crawler {

namespace components {

class IFrontPrioritizer {
 public:
  virtual std::pair<types::URL, std::size_t> selectQueue(
      const std::string& url) = 0;
  virtual ~IFrontPrioritizer() = default;
};

}  // namespace components

}  // namespace crawler