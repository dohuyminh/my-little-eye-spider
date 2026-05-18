#pragma once

#include <concepts>

#include "types/url.h"

namespace crawler::components {

template <typename T>
concept ValidQueueDataType = requires(T t) {
  { t.url() } -> std::same_as<const types::URL&>;
};

}  // namespace crawler::components
