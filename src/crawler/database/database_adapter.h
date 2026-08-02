#pragma once

#include <exception>
#include <expected>
namespace crawler::database {

template <typename ReturnType>
using DbQueryReturn = std::expected<ReturnType, std::exception_ptr>;

}  // namespace crawler::database
