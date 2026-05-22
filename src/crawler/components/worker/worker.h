#pragma once

#include <memory>
#include <type_traits>
#include <utility>

#include "../frontier/frontier.h"
#include "moodycamel/concurrentqueue.h"

namespace crawler {

namespace components {

template <typename Test, template <typename...> typename Ref>
struct IsSpecialization : std::false_type {};

template <template <typename...> typename Ref, typename... Args>
struct IsSpecialization<Ref<Args...>, Ref> : std::true_type {};

template <typename T>
concept FrontierType = IsSpecialization<T, Frontier>::value;

template <FrontierType F>
class Worker {
 public:
  Worker(
      std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs,
      F& frontier)
      : producingURLs_(producingURLs), frontier_(frontier) {}

  virtual void doWork() = 0;

  void assignData(typename F::DataType&& data) { data_ = std::move(data); }

 protected:
  F::DataType data_;

  std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> producingURLs_;

  F& frontier_;
};

}  // namespace components

}  // namespace crawler
