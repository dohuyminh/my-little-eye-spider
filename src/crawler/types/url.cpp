#include "url.h"

#include <numeric>
#include <stdexcept>

namespace crawler {

namespace types {

URL::URL(std::string&& url) {
  services::url::ParseResult<services::url::URLParseResult> result{services::url::parse(url)};

  if (result.has_value()) {
    userinfo_ = std::move(result->userinfo);
    scheme_ = std::move(result->scheme);
    subdomains_ = std::move(result->subdomains);
    domain_ = std::move(result->domain);
    port_ = std::move(result->port);
    path_ = std::move(result->path);
    queryParams_ = std::move(result->queryParams);
    fragment_ = std::move(result->fragment);
  } else {
    throw std::invalid_argument("Invalid URL: " + url);
  }
}

URL::URL(const std::string& url) {
  services::url::ParseResult<services::url::URLParseResult> result{services::url::parse(url)};

  if (result.has_value()) {
    userinfo_ = std::move(result->userinfo);
    scheme_ = std::move(result->scheme);
    subdomains_ = std::move(result->subdomains);
    domain_ = std::move(result->domain);
    port_ = std::move(result->port);
    path_ = std::move(result->path);
    queryParams_ = std::move(result->queryParams);
    fragment_ = std::move(result->fragment);
  } else {
    throw std::invalid_argument("Invalid URL: " + url);
  }
}

std::string URL::path() const noexcept {
  return std::accumulate(path_.begin(), path_.end(), std::string());
}

std::string URL::to_string() const {
  std::string result;

  // scheme://
  if (!scheme_.empty()) {
    result += scheme_ + "://";
  }

  // userinfo@
  if (!userinfo_.empty()) {
    result += userinfo_ + "@";
  }

  // domain
  result += domain_;

  // :port
  if (!port_.empty()) {
    result += ":" + port_;
  }

  // path
  result += path();

  // ?query
  if (!queryParams_.empty()) {
    result += "?";
    bool first = true;
    for (const auto& [key, value] : queryParams_) {
      if (!first) result += "&";
      result += key + "=" + value;
      first = false;
    }
  }

  // #fragment
  if (!fragment_.empty()) {
    result += "#" + fragment_;
  }

  return result;
}

}  // namespace types

}  // namespace crawler
