#pragma once

#include <curl/curl.h>

namespace crawler {

namespace services {

namespace curl {

class CURLEasyHandler {
 public:
  static CURLEasyHandler& get();

  CURL* handler() const noexcept;

  ~CURLEasyHandler();

 private:
  CURLEasyHandler();

  CURL* handler_;

 public:
  // copy constructor/operator is forbidden
  CURLEasyHandler(const CURLEasyHandler& other) = delete;
  void operator=(const CURLEasyHandler& other) = delete;
};

}  // namespace curl

}  // namespace services

}  // namespace crawler