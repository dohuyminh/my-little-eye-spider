#pragma once

#include <cstdint>
#include <string>

namespace crawler {

namespace services {

namespace curl {

using ResponseCode = uint32_t;

class Response {
 public:
  Response(const std::string& content, ResponseCode code) noexcept;

  const std::string& content() const noexcept { return content_; }

  ResponseCode code() const noexcept { return code_; }

 private:
  std::string content_;
  ResponseCode code_;
};

}  // namespace curl

}  // namespace services

}  // namespace crawler