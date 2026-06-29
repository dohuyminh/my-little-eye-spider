#pragma once

#include <expected>
namespace crawler::database {

enum class ExecutionStatus {
  EXEC_OK,
  EXEC_FAIL,
};

template <typename ReturnType>
using DbQueryReturn = std::expected<ReturnType, ExecutionStatus>;

}  // namespace crawler::database
