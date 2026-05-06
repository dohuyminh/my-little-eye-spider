#pragma once

#include <concepts>

namespace crawler {

namespace types {

template <typename T>
concept HasStatefulFlag = requires {
  { T::isStateful } -> std::convertible_to<bool>;
};

template <typename T>
concept Stateful = HasStatefulFlag<T> && T::isStateful;

template <typename T>
concept Stateless = HasStatefulFlag<T> && !T::isStateful;

}  // namespace types

}  // namespace crawler