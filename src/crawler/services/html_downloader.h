#pragma once

#include <curl/curl.h>

#include <istream>
#include <stdexcept>

#include "curl_response.h"
#include "types/url.h"

namespace crawler {

namespace services {

namespace html {

class Downloader {
 public:
  curl::Response operator()(const types::URL& url);

 private:
  static size_t writeCallback(void* contents, size_t size, size_t nmemb,
                              std::string* out);
};

}  // namespace html

}  // namespace services

}  // namespace crawler