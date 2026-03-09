#pragma once

#include <cstddef>
#include <stdexcept>

namespace memory {

/**
 * @brief Arena: A simple memory arena for fast allocation and deallocation.
 *
 * The Arena class provides a memory pool from which memory can be allocated
 * in a linear fashion. It allows for fast allocation of objects and bulk
 * deallocation by resetting the arena. This is useful for scenarios where
 * many small allocations are needed and can be freed all at once.
 *
 */
class Arena {
 public:
  Arena(std::size_t capacity);
  ~Arena();

  bool canAllocate() const noexcept {
    return buffer_ != nullptr && offset_ < capacity_;
  }

  std::size_t capacity() const noexcept { return capacity_; }

  std::size_t occupiedMemory() const noexcept { return offset_; }

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

  template <typename T, typename... Args>
  T* allocate(Args&&... args) {
    std::size_t typeSize = sizeof(T);
    if (offset_ + typeSize > capacity_) {
      throw std::bad_alloc();
    }

    T* obj = new (buffer_ + offset_) T(std::forward<Args>(args)...);
    offset_ += typeSize;
    return obj;
  }

 private:
  std::byte* buffer_{nullptr};
  std::size_t capacity_{0};
  std::size_t offset_{0};
};

}  // namespace memory