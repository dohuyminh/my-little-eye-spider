#pragma once

#include <type_traits>
#include <variant>
#include "database/models/metadata.h"
#include "url.h"
#include "url_contents.h"
#include "url_metadata.h"
#include "robots_txt.h"

namespace crawler::database {

using Model = std::variant<
  URL, 
  URLContent, 
  URLMetadata, 
  RobotsTxtEntry
>;

template <typename T>
concept AbstractModel = std::is_same_v<T, PostgresModel>;

template <AbstractModel M>
M& getUnderlyingModel(Model& model) {
  return std::visit([](M& arg) -> M& { return arg; }, model);
}

template <AbstractModel M>
const M& getUnderlyingModel(const Model& model) {
  return std::visit([](const M& arg) -> const M& { return arg; }, model);
}

}
