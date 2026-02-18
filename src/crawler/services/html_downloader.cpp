#include "html_downloader.h"

#include <format>

#include "curl_handler.h"

namespace crawler {

namespace services {

namespace html {

curl::Response Downloader::operator()(const types::URL& url) {
  // get thread's cURL handler to perform GET requests
  CURL* handler = curl::CURLEasyHandler::get().handler();
  if (!handler) {
    throw std::runtime_error("CURL: handler access failed");
  }

  // perform GET request
  std::string response;
  curl_easy_setopt(handler, CURLOPT_URL, url.url().c_str());
  curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, &Downloader::writeCallback);
  curl_easy_setopt(handler, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(handler, CURLOPT_TIMEOUT, 10L);

  CURLcode res = curl_easy_perform(handler);
  if (res != CURLE_OK) {
    throw std::runtime_error(std::format(
        "CURL: performing GET request failed: {}", curl_easy_strerror(res)));
  }

  curl::ResponseCode code;
  curl_easy_getinfo(handler, CURLINFO_RESPONSE_CODE, &code);

  return curl::Response(response, code);
}

size_t Downloader::writeCallback(void* contents, size_t size, size_t nmemb,
                                 std::string* output) {
  size_t total = size * nmemb;
  output->append(static_cast<char*>(contents), total);
  return total;
}

}  // namespace html

}  // namespace services

}  // namespace crawler