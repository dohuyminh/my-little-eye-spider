#pragma once

#include <concepts>
#include <type_traits>

#include "types/url.h"

namespace crawler::components {

template <typename T>
concept ValidQueueDataType =
    std::is_default_constructible_v<T> && requires(T t) {
      { t.url() } -> std::same_as<const types::URL&>;
    };

class StatelessQDT {
 public:
  StatelessQDT() = default;

  StatelessQDT(const types::URL& url) : url_(url) {}

  const types::URL& url() const noexcept { return url_; }

 private:
  types::URL url_{};
};

static_assert(
    ValidQueueDataType<StatelessQDT>,
    "[valid_queue_data_type.h] StatelessQDT does not pass the constraint");

}  // namespace crawler::components
