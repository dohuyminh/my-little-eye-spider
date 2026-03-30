#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "services/url.h"

namespace crawler {

namespace types {

class URL {
 public:
  URL() = default;

  URL(std::string&& url);

  URL(const std::string& url);

  // copy constructor
  URL(const URL& other) = default;

  URL& operator=(const URL& other) = default;

  // move constructor
  URL(URL&& other) = default;

  URL& operator=(URL&& other) = default;

  const std::string& url() const noexcept { return url_; }

  const std::string& scheme() const noexcept { return scheme_; }

  const std::vector<std::string>& subdomains() const noexcept {
    return subdomains_;
  }

  const std::string& domain() const noexcept { return domain_; }

  const std::string& port() const noexcept { return port_; }

  std::string path() const noexcept;

  std::string root() const {
    std::string schemePrefix{scheme_.empty() ? "" : scheme_ + "://"};
    return schemePrefix + domain_ + "/";
  }

  std::optional<std::string> query(const std::string& key) const noexcept {
    auto it{queryParams_.find(key)};
    if (it != queryParams_.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  const std::string& fragment() const noexcept { return fragment_; }

  bool applyPath(const std::string& relativeURL) {
    return services::url::parseAndApplyRelativeURL(relativeURL, path_); 
  }

  std::string to_string() const;

 private:
  std::string url_;

  std::string userinfo_;
  std::string scheme_;
  std::vector<std::string> subdomains_;
  std::string domain_;
  std::string port_;
  std::vector<std::string> path_;
  std::unordered_map<std::string, std::string> queryParams_;
  std::string fragment_;
};

}  // namespace types

}  // namespace crawler
