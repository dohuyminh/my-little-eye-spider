#include "curl_handler.h"

namespace crawler {

namespace services {

namespace curl {

CURLEasyHandler& CURLEasyHandler::get() {
  static thread_local CURLEasyHandler instance;
  return instance;
}

CURLEasyHandler::CURLEasyHandler() : handler_(curl_easy_init()) {}

CURLEasyHandler::~CURLEasyHandler() { curl_easy_cleanup(handler_); }

CURL* CURLEasyHandler::handler() const noexcept { return handler_; }

}  // namespace curl

}  // namespace services

}  // namespace crawler