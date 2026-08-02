#pragma once

#include <atomic>
#include <bit>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <type_traits>
#include <vector>

namespace crawler::services::concurrency {

template <typename T>
class LockFreeRingBuffer {
public:
 
  explicit LockFreeRingBuffer(std::size_t capacity) {
    capacity = std::bit_ceil(capacity); 
    buffer_ = new T[capacity];
    capacity_ = capacity;
  }
  
  template <typename ...Args>
  bool emplace(Args&&... args)
    requires (std::is_constructible_v<T, Args&&...>)
  {
    std::size_t writePtr = writePtr_.load(std::memory_order_relaxed); 
    if (full(cachedReadPtr_, writePtr)) {
      cachedReadPtr_ = readPtr_.load(std::memory_order_acquire);
      if (full(cachedReadPtr_, writePtr)) {
        return false;
      }
    }

    new(&buffer_[writePtr & (capacity_ - 1)]) T(std::forward<Args>(args)...);
    writePtr_.store(writePtr + 1, std::memory_order_release);

    return true;
  } 

  bool push(const T& value)
    requires (std::is_copy_constructible_v<T>)
  {
    std::size_t writePtr = writePtr_.load(std::memory_order_relaxed); 
    if (full(cachedReadPtr_, writePtr)) {
      cachedReadPtr_ = readPtr_.load(std::memory_order_acquire);
      if (full(cachedReadPtr_, writePtr)) {
        return false;
      }
    }

    new(&buffer_[writePtr & (capacity_ - 1)]) T(value);
    writePtr_.store(writePtr + 1, std::memory_order_release);
    
    return true;
  }

  bool pop(T& ref)
    requires (std::is_default_constructible_v<T> && std::is_copy_assignable_v<T>)
  {
    std::size_t readPtr = readPtr_.load(std::memory_order_relaxed);
    if (empty(readPtr, cachedWritePtr_)) {
      cachedWritePtr_ = writePtr_.load(std::memory_order_acquire);
      if (empty(readPtr, cachedWritePtr_)) {
        return false;
      }
    }

    ref = buffer_[readPtr & (capacity_ - 1)];
    buffer_[readPtr & (capacity_ - 1)].~T();
    readPtr_.store(readPtr + 1, std::memory_order_release);
    return true;
  }

  std::vector<T> popBatch(std::size_t batchSize = static_cast<std::size_t>(-1))
    requires (std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>)
  {
    std::size_t readPtr = readPtr_.load(std::memory_order_relaxed);
    if (empty(readPtr, cachedWritePtr_)) {
      cachedWritePtr_ = writePtr_.load(std::memory_order_acquire);
      if (empty(readPtr, cachedWritePtr_)) {
        return {};
      }
    }

    std::vector<T> res;
    std::size_t size{cachedWritePtr_ - readPtr}, count{
      batchSize == static_cast<std::size_t>(-1) ? 
        size : 
        std::min(batchSize, size)};
    res.reserve(count);

    for (std::size_t i{}; i < count; ++i) {
      std::size_t idx{(readPtr + i) & (capacity_ - 1)};
      res.emplace_back(std::move(buffer_[idx]));
      buffer_[idx].~T();
    }

    readPtr_.store(readPtr_ + count, std::memory_order_release);

    return res;
  }

  std::size_t capacity() const noexcept {
    return capacity_;
  }

  std::size_t size() const {
    return writePtr_.load(std::memory_order_acquire) - readPtr_.load(std::memory_order_acquire);
  }
  
  ~LockFreeRingBuffer() {
    std::size_t readPtr = readPtr_.load(std::memory_order_acquire), 
                writePtr = writePtr_.load(std::memory_order_acquire);
    
    while (readPtr < writePtr) {
      buffer_[readPtr & (capacity_ - 1)].~T();
      ++readPtr;
    }

    delete[] buffer_;
  }

private:

  constexpr static std::size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
  
  std::size_t capacity_;
  T* buffer_;

  alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> readPtr_{};
  alignas(CACHE_LINE_SIZE) std::size_t cachedWritePtr_{};
  
  alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> writePtr_{};
  alignas(CACHE_LINE_SIZE) std::size_t cachedReadPtr_{};
  
  bool full(std::size_t readPtr, std::size_t writePtr) {
    return writePtr - readPtr == capacity_;
  }

  bool empty(std::size_t readPtr, std::size_t writePtr) {
    return readPtr == writePtr;
  }
};

}
