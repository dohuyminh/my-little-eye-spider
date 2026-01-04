#include "arena.h"

namespace memory {

Arena::Arena(std::size_t capacity) : capacity_(capacity) {
    buffer_ = new std::byte[capacity_];
}

Arena::~Arena() {
    delete[] buffer_;
}

}