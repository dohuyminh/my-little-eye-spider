#pragma once

#include <concepts>
#include <expected>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
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
  FRAGMENT_ERROR,
  ROBOTS_TXT_ERROR
};

struct URLParseResult {
  std::string scheme;
  std::string userinfo;
  std::vector<std::string> subdomains;
  std::string domain;
  std::string port;
  std::vector<std::string> path;
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string encodedURL;

  URLParseResult() = default;
};

template <typename ResultType>
using ParseResult = std::expected<ResultType, ParseError>;

// Template concept for string-like types
template <typename T>
concept StringLike = requires(const T& t) {
  { t.data() } -> std::convertible_to<const char*>;
  { t.size() } -> std::convertible_to<std::size_t>;
};

// Percent encoding/decoding (defined in url.cpp, must be inline for templates)
inline std::string percentEncode(const StringLike auto& src,
                                 char ignore = '\0');
inline std::string percentDecode(const StringLike auto& src);
inline bool isPercentEncoded(const StringLike auto& str);

constexpr bool isAscii(char c) { return static_cast<unsigned char>(c) <= 127; }

bool parseAndApplyRelativeURL(
    const std::string& relativeURL, std::vector<std::string>& path,
    std::unordered_map<std::string, std::string>& queryParams,
    std::string& fragment);

/**
 * Validates if a string is a valid relative URL.
 * Returns true for relative URLs (e.g., "/path", "page.html", "../parent").
 * Returns false for absolute URLs (with scheme) or invalid strings.
 */
bool isRelativeURL(std::string_view url);

ParseResult<URLParseResult> parse(const std::string& url);

using RobotsTxtRepr = nlohmann::json;

ParseResult<RobotsTxtRepr> parseRobotsTxt(const std::string& content);

bool urlIsDisallowed(const std::string& url, const std::string& pattern);

}  // namespace url

}  // namespace services

}  // namespace crawler

// Template definitions must be in header
namespace crawler {

namespace services {

namespace url {

inline std::string percentEncode(const StringLike auto& src, char ignore) {
  std::string result;
  result.reserve(src.size() * 3);

  constexpr char hexStr[]{"0123456789ABCDEF"};

  for (unsigned char c : src) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
        c == '~' || c == ignore) {
      result.push_back(c);
    } else {
      result.push_back('%');
      result.push_back(hexStr[c >> 4]);
      result.push_back(hexStr[c & 0xF]);
    }
  }
  return result;
}

inline std::string percentDecode(const StringLike auto& src) {
  std::string result;
  result.reserve(src.size());

  auto hexToDec = [](std::string_view hexView) -> char {
    if (hexView.size() != 2) {
      throw std::runtime_error("[PERCENT DECODING]: cannot parse hex value");
    }
    static int lookup[256];
    static bool initialized = []() {
      for (auto i = 0; i < 256; ++i) lookup[i] = -1;
      for (char c = '0'; c <= '9'; ++c)
        lookup[static_cast<unsigned char>(c)] = c - '0';
      for (char c = 'A'; c <= 'F'; ++c)
        lookup[static_cast<unsigned char>(c)] = c - 'A' + 10;
      for (char c = 'a'; c <= 'f'; ++c)
        lookup[static_cast<unsigned char>(c)] = c - 'a' + 10;
      return true;
    }();
    (void)initialized;

    int high = lookup[static_cast<unsigned char>(hexView[0])];
    int low = lookup[static_cast<unsigned char>(hexView[1])];
    if (high == -1 || low == -1) {
      throw std::runtime_error("[PERCENT DECODING]: cannot parse hex value");
    }
    return static_cast<char>((high << 4) | low);
  };

  for (auto ptr = 0ULL; ptr < src.size();) {
    if (src[ptr] == '%' && ptr + 2 < src.size()) {
      result.push_back(hexToDec(std::string_view(src.data() + ptr + 1, 2)));
      ptr += 3;
      continue;
    }
    result.push_back(src[ptr]);
    ++ptr;
  }

  return result;
}

inline bool isPercentEncoded(const StringLike auto& str) {
  return percentDecode(str) != str;
}

constexpr bool isUnreserved(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~';
}

constexpr bool isSubdelim(char c) {
  return c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' ||
         c == ')' || c == '*' || c == '+' || c == ',' || c == ';' || c == '=';
}

constexpr bool isPChar(char c) {
  return isUnreserved(c) || isSubdelim(c) || c == ':' || c == '@';
}

constexpr bool isPCharNC(char c) {
  return isUnreserved(c) || isSubdelim(c) || c == '@';
}

// Helper functions for relative URL parsing
/**
 * Validate the path component of a relative reference
 * Returns false if path starts with "//" (not supported)
 */
bool validatePathComponent(std::string_view pathStr);

/**
 * Apply path component to existing path vector
 * Handles ".", "..", and normal segments
 */
bool applyPathComponent(std::string_view pathStr,
                        std::vector<std::string>& path);

/**
 * Validate query component syntax
 */
bool validateQueryComponent(std::string_view queryStr);

/**
 * Apply query component (parse key-value pairs)
 */
bool applyQueryComponent(
    std::string_view queryStr,
    std::unordered_map<std::string, std::string>& queryParams);

/**
 * Validate fragment component syntax
 */
bool validateFragmentComponent(std::string_view fragmentStr);

/**
 * Apply fragment component
 */
bool applyFragmentComponent(std::string_view fragmentStr,
                            std::string& fragment);

/**
 * Helper: Validate authority component (userinfo@host:port)
 */
bool validateAuthorityComponent(std::string_view authorityStr);

/**
 * Helper: Apply authority component (parse and store host, port, userinfo)
 */
bool applyAuthorityComponent(std::string_view authorityStr, std::string& host,
                             std::string& port, std::string& userinfo);

/**
 * Structure for parsed relative URL components
 * Includes optional authority fields for network-path-reference format (//authority)
 */
struct RelativeURLComponents {
  std::string_view path;
  std::string_view query;
  std::string_view fragment;
  std::string_view authority;  // For network-path-reference (//authority)
  bool isValid;
};

/**
 * Parse relative URL into components and validate each
 */
RelativeURLComponents parseRelativeURLComponents(
    const std::string& relativeURL);

/**
 * Extended version of parseAndApplyRelativeURL with authority support
 * Handles network-path-reference format (//authority path-abempty)
 */
bool parseAndApplyRelativeURL(
    const std::string& relativeURL, std::vector<std::string>& path,
    std::unordered_map<std::string, std::string>& queryParams,
    std::string& fragment, std::string& host, std::string& port,
    std::string& userinfo);

}  // namespace url

}  // namespace services

}  // namespace crawler
