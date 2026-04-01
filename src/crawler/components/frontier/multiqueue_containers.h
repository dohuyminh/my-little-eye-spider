#pragma once

#include <exception>
#include <concepts>
#include <moodycamel/concurrentqueue.h>
#include <optional>
#include <queue>
#include <ranges>
#include <vector>
#include <iterator>

namespace crawler {

namespace components {

template <typename T>
concept QueueType = requires(T t) {
  // ensures T has a value_type member type
  typename T::value_type;

  // single-element enqueue accepts universal references (both lvalue and rvalue)
  { t.enqueue(std::declval<const typename T::value_type&>()) } -> std::same_as<void>;
  { t.enqueue(std::declval<typename T::value_type&&>()) } -> std::same_as<void>;

  // single-element dequeue
  { t.dequeue() } -> std::convertible_to<std::optional<typename T::value_type>>;

  // bulk enqueue accepts universal references to vector containers
  { t.enqueue_bulk(std::declval<const std::vector<typename T::value_type>&>()) } -> std::same_as<void>;
  { t.enqueue_bulk(std::declval<std::vector<typename T::value_type>&&>()) } -> std::same_as<void>;

  // bulk dequeue returns a vector of value_type
  { t.dequeue_bulk(std::declval<std::size_t>()) } -> std::convertible_to<std::vector<typename T::value_type>>;
};

template <QueueType Queue>
class MultiQueueContainers {
public:
  using ValueType = typename Queue::value_type;

  MultiQueueContainers(std::size_t numQueues) : queues_(numQueues) {}

  void enqueue(std::size_t queueIndex, const ValueType& value) {
    if (queueIndex >= queues_.size()) {
      throw std::invalid_argument("Invalid queue index");
    }
    queues_[queueIndex].enqueue(value);
  }

  void enqueue(std::size_t queueIndex, ValueType&& value) {
    if (queueIndex >= queues_.size()) {
      throw std::invalid_argument("Invalid queue index");
    }
    queues_[queueIndex].enqueue(std::forward<ValueType>(value));
  }

  std::optional<ValueType> dequeue(std::size_t queueIndex) {
    if (queueIndex >= queues_.size()) {
      throw std::invalid_argument("Invalid queue index");
    }
    return queues_[queueIndex].dequeue();
  }

  void enqueue_bulk(std::size_t queueIndex, const std::vector<ValueType>& values) {
    if (queueIndex >= queues_.size()) {
      throw std::invalid_argument("Invalid queue index");
    }
    queues_[queueIndex].enqueue_bulk(values);
  }

  void enqueue_bulk(std::size_t queueIndex, std::vector<ValueType>&& values) {
    if (queueIndex >= queues_.size()) {
      throw std::invalid_argument("Invalid queue index");
    }
    queues_[queueIndex].enqueue_bulk(std::forward<std::vector<ValueType>>(values));
  }

  std::vector<ValueType> dequeue_bulk(std::size_t queueIndex, std::size_t count) {
    if (queueIndex >= queues_.size()) {
      throw std::invalid_argument("Invalid queue index");
    }
    return queues_[queueIndex].dequeue_bulk(count);
  }

private:
  std::vector<Queue> queues_;
};

template <typename T>
class StdQueueWrapper {
public:
  using value_type = T;

  StdQueueWrapper() = default;

  void enqueue(const T& value) {
    queue_.push(value);
  }

  void enqueue(T&& value) {
    queue_.push(std::move(value));
  }

  std::optional<T> dequeue() {
    if (queue_.empty()) {
      return std::nullopt;
    }
    T value = std::move(queue_.front());
    queue_.pop();
    return std::make_optional(std::move(value));
  }

  void enqueue_bulk(const std::vector<T>& values) {
    for (const auto& value : values) {
      enqueue(value);
    }
  }

  void enqueue_bulk(std::vector<T>&& values) {
    for (auto&& value : values) {
      enqueue(std::move(value));
    }
  }

  std::vector<T> dequeue_bulk(std::size_t count) {
    std::vector<T> result;
    result.reserve(count);
    for (std::size_t i{0}; i < count && !queue_.empty(); ++i) {
      result.push_back(std::move(queue_.front()));
      queue_.pop();
    }
    return result;
  }

private:
  std::queue<T> queue_;
};

template <typename T>
class MoodyCamelConcurrentQueueWrapper {
public:
  using value_type = T;

  MoodyCamelConcurrentQueueWrapper() = default;

  void enqueue(const T& value) {
    queue_.enqueue(value);
  }

  void enqueue(T&& value) {
    queue_.enqueue(std::move(value));
  }

  std::optional<T> dequeue() {
    T value;
    if (queue_.try_dequeue(value)) {
      return std::make_optional(std::move(value));
    }
    return std::nullopt;
  }

  void enqueue_bulk(const std::vector<T>& values) {
    for (const auto& value : values) {
      enqueue(value);
    }
  }

  void enqueue_bulk(std::vector<T>&& values) {
    for (auto&& value : values) {
      enqueue(std::move(value));
    }
  }

  std::vector<T> dequeue_bulk(std::size_t count) {
    std::vector<T> result;
    result.reserve(count);
    queue_.try_dequeue_bulk(std::back_inserter(result), count);
    return result;
  }

private:
  moodycamel::ConcurrentQueue<T> queue_;
};

} // namespace components

} // namespace crawler
