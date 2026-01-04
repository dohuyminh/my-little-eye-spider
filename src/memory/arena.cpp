#include "arena.h"

#include <cstring>

namespace memory {

Arena::Arena(std::size_t capacity) : capacity_(capacity) {
    buffer_ = new std::byte[capacity_];
}

Arena::~Arena() {
    delete[] buffer_;
}

Arena::Arena(Arena&& other) noexcept
    : buffer_(other.buffer_), capacity_(other.capacity_), offset_(other.offset_) {
    other.buffer_ = nullptr;
    other.capacity_ = 0;
    other.offset_ = 0;
}

Arena& Arena::operator=(Arena&& other) noexcept {
    if (this != &other) {
        delete[] buffer_;
        buffer_ = other.buffer_;
        capacity_ = other.capacity_;
        offset_ = other.offset_;
        other.buffer_ = nullptr;
        other.capacity_ = 0;
        other.offset_ = 0;
    }
    return *this;
}

}