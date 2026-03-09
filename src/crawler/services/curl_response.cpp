#include "curl_response.h"

namespace crawler {

namespace services {

namespace curl {

Response::Response(const std::string& content, ResponseCode code) noexcept
    : content_(content), code_(code) {}

}  // namespace curl

}  // namespace services

}  // namespace crawler
