#pragma once

#include <expected>
#include <string>
#include <unordered_map>
#include <vector>

namespace crawler {

namespace services {

namespace url {

enum class ParseError {
  INVALID_URL,
  SCHEME_ERROR,
  AUTHORITY_ERROR,
  PATH_ERROR,
  QUERY_ERROR,
  FRAGMENT_ERROR
};

struct ParseInfo {
  std::string scheme;
  std::string userinfo;
  std::vector<std::string> subdomains;
  std::string domain;
  std::string port;
  std::string path;
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;

  ParseInfo() = default;
};

using ParseResult = std::expected<ParseInfo, ParseError>;

ParseResult parse(const std::string& url);

}  // namespace url

}  // namespace services

}  // namespace crawler
