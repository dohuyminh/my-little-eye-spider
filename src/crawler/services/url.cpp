#include "url.h"

#include <sched.h>

#include <array>
#include <cctype>
#include <cstddef>
#include <mutex>
#include <stdexcept>
#include <string_view>

namespace crawler {

namespace services {

namespace url {

bool parseAndApplyRelativeURL(const std::string& relativeURL,
                      std::vector<std::string>& path) {
  if (!isRelativeURL(relativeURL.c_str())) {
    return false;
  }

  if (relativeURL.empty()) {
    return true;  // nothing to change
  }

  std::vector<std::string> newPath = path;  // start with current path

  size_t pos = 0;

  // Check if the relative URL starts with '/'
  if (relativeURL[0] == '/') {
    newPath.clear();  // start from root
    newPath.emplace_back("/");
    pos = 1;  // skip the leading slash
  }

  while (pos < relativeURL.size()) {
    // Find next slash
    size_t next{relativeURL.find('/', pos)};
    bool backFlag{false};
    std::string_view segment;
    if (next == std::string::npos) {
      segment = std::string_view(relativeURL).substr(pos);
      pos = relativeURL.size();  // will exit loop
    } else {
      segment = std::string_view(relativeURL).substr(pos, next - pos);
      pos = next + 1;
      backFlag = true;
    }

    // Handle special segments
    if (segment.empty()) {
      // Empty segment due to multiple slashes – ignore
      continue;
    }
    if (segment == ".") {
      // Current directory – do nothing
      continue;
    }
    if (segment == "..") {
      // Parent directory – pop if possible
      if (newPath.empty() || (newPath.size() == 1 && newPath.back() == "/")) {
        return false;  // cannot go above root
      }
      newPath.pop_back();
      continue;
    }

    // Normal segment: encode if not already percent-encoded
    if (isPercentEncoded(segment)) {
      newPath.emplace_back(segment);
    } else {
      newPath.emplace_back(percentEncode(segment));
    }

    if (backFlag) {
      newPath.back().push_back('/');
    }
  }

  path = std::move(newPath);
  return true;
}

bool isRelativeURL(std::string_view url) {
  if (url.empty()) {
    return true;  // empty string is technically valid (no-op relative URL)
  }

  // Check for scheme pattern: letter+ followed by "://"
  // RFC 3986: scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
  size_t colonPos{url.find(':')};

  // No colon means no scheme, so it's relative
  if (colonPos == std::string_view::npos) {
    return true;
  }

  // Check if it's followed by "//" (scheme-specific part delimiter)
  if (colonPos + 2 >= url.size() || url[colonPos + 1] != '/' ||
      url[colonPos + 2] != '/') {
    // Just "foo:bar" without "//" - could be a colon in path, not a scheme
    // Check if all chars before colon are valid scheme chars
    if (colonPos == 0) {
      return true;  // starts with ":" - not a scheme, treat as relative
    }

    // Validate scheme characters
    for (size_t i = 0; i < colonPos; ++i) {
      char c = url[i];
      if (i == 0) {
        // First char must be ALPHA
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
          return true;  // not a valid scheme start, so relative
        }
      } else {
        // Subsequent chars: ALPHA / DIGIT / "+" / "-" / "."
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.')) {
          return true;  // invalid scheme char, so not a scheme
        }
      }
    }
    // All chars before colon are valid scheme chars, but no "://" follows
    // This is ambiguous but likely not an absolute URL
    return true;
  }

  // We have "://" - verify the scheme part is valid
  if (colonPos == 0) {
    return true;  // starts with "://" - protocol-relative URL
  }

  // Validate scheme characters
  for (size_t i = 0; i < colonPos; ++i) {
    char c = url[i];
    if (i == 0) {
      // First char must be ALPHA
      if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
        return true;  // not a valid scheme start
      }
    } else {
      // Subsequent chars: ALPHA / DIGIT / "+" / "-" / "."
      if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.')) {
        return true;  // invalid scheme char
      }
    }
  }

  // Has valid scheme followed by "://" - this is an absolute URL
  return false;
}

ParseResult<URLParseResult> parse(const std::string& url)  {
  URLParseResult result;

  constexpr char pattern[] =
      R"(^(?:(https?|ftp):\/\/)?([^/?#]+)([^?#]*)(?:\?([^#]*))?(?:#(.*))?$)";

  std::string_view queryString;
  std::string_view authority;
  std::string_view path;

  if (!RE2::FullMatch(url, pattern, &result.scheme, &authority, &path,
                      &queryString, &result.fragment)) {
    return std::unexpected(ParseError::INVALID_URL);
  }

  // validate authority
  if (authority.empty()) {
    return std::unexpected(ParseError::AUTHORITY_ERROR);
  }

  // decompose URL into 3 components: [userinfo +] host [+ port]
  std::string_view userinfo, host, port;
  constexpr char authorityPattern[]{
      R"(^(([^@]*)@)?(\[[0-9a-fA-F:.]+\]|[^/?#:@]+)(:(\d+))?$)"};
  if (!RE2::FullMatch(authority, authorityPattern, nullptr, &userinfo, &host,
                      nullptr, &port)) {
    return std::unexpected(ParseError::INVALID_URL);
  }

  // userinfo
  if (!userinfo.empty()) {
    try {
      std::string_view username, password;

      std::size_t colonPos{userinfo.find(':')};
      bool supportedFormat{false};

      // username:password format
      if (colonPos != std::string::npos) {
        username = userinfo.substr(0, colonPos);
        password = userinfo.substr(colonPos + 1);
        supportedFormat = true;
      }
      // other userinfo format (unsupported)
      else {
        username = userinfo;
      }

      std::string uStr{username}, pStr{password};
      std::string decodedUsername{percentDecode(uStr)},
          decodedPassword{percentDecode(pStr)};

      result.userinfo.append(percentEncode(decodedUsername));
      if (supportedFormat) {
        result.userinfo.push_back(':');
        result.userinfo.append(percentEncode(decodedPassword));
      }
    } catch (std::runtime_error const& e) {
      return std::unexpected(ParseError::AUTHORITY_ERROR);
    }
  }

  // port
  if (!port.empty()) {
    result.port = port;
  }

  // ensure host is not actually empty
  if (host.empty()) {
    return std::unexpected(ParseError::AUTHORITY_ERROR);
  }

  // normalize authority (handle IDN conversion and port extraction)
  std::string hostStr(host);

  // Check if host part contains non-ASCII (IDN domain)
  bool hasNonAscii = std::any_of(hostStr.begin(), hostStr.end(),
                                 [](char c) { return !isAscii(c); });

  // Convert IDN to ASCII (punycode) if needed
  if (hasNonAscii) {
    char* asciiOutput{nullptr};
    int rc{idn2_to_ascii_8z(hostStr.c_str(), &asciiOutput, 0)};
    if (rc == IDN2_OK && asciiOutput != nullptr) {
      hostStr = std::string(asciiOutput);
      idn2_free(asciiOutput);
    } else {
      return std::unexpected(ParseError::AUTHORITY_ERROR);
    }
  }

  // Validate host format (regex) - now on normalized host
  constexpr char hostPattern[] =
      R"(^[a-zA-Z0-9:._\-]+$)";  // TODO: IMPLEMENT A STRICTER REGEX
  if (!RE2::FullMatch(hostStr, hostPattern)) {
    return std::unexpected(ParseError::AUTHORITY_ERROR);
  }

  // Split authority into labels by '.'
  std::vector<std::string> labels;
  std::string label;
  for (char c : hostStr) {
    if (c == '.') {
      if (!label.empty()) {
        labels.push_back(label);
        label.clear();
      }
    } else {
      label += c;
    }
  }
  if (!label.empty()) {
    labels.push_back(label);
  }

  // Extract domain and subdomains using libpsl
  const psl_ctx_t* psl{psl_builtin()};
  if (psl != nullptr) {
    const char* registeredDomain{psl_registrable_domain(psl, hostStr.c_str())};

    if (registeredDomain != nullptr) {
      result.domain = registeredDomain;
      std::string registeredDomainStr(registeredDomain);

      // Extract subdomains: everything before the registered domain
      if (hostStr.length() > registeredDomainStr.length()) {
        size_t domainStart{hostStr.rfind(registeredDomainStr)};
        if (domainStart > 0 && hostStr[domainStart - 1] == '.') {
          std::string subdomainPart{hostStr.substr(0, domainStart - 1)};

          // Split subdomains by '.'
          std::string subdomain;
          for (char c : subdomainPart) {
            if (c == '.') {
              if (!subdomain.empty()) {
                result.subdomains.push_back(subdomain);
                subdomain.clear();
              }
            } else {
              subdomain += c;
            }
          }
          if (!subdomain.empty()) {
            result.subdomains.push_back(subdomain);
          }
        }
      }
    } else {
      // Fallback: libpsl couldn't identify registered domain
      // Use simple 2-label heuristic
      if (labels.size() >= 2) {
        result.domain =
            labels[labels.size() - 2] + "." + labels[labels.size() - 1];
        for (size_t i = 0; i < labels.size() - 2; ++i) {
          result.subdomains.push_back(labels[i]);
        }
      } else if (labels.size() == 1) {
        result.domain = labels[0];
      }
    }
  } else {
    // Fallback: libpsl unavailable
    if (labels.size() >= 2) {
      result.domain =
          labels[labels.size() - 2] + "." + labels[labels.size() - 1];
      for (size_t i = 0; i < labels.size() - 2; ++i) {
        result.subdomains.push_back(labels[i]);
      }
    } else if (labels.size() == 1) {
      result.domain = labels[0];
    }
  }

  // parse path
  bool pathParse{parseAndApplyRelativeURL(path.data(), result.path)};
  if (!pathParse) {
    return std::unexpected(ParseError::PATH_ERROR);
  }

  // parse query parameters
  constexpr char queryParamPattern[]{R"(([^=&]+)=([^&]*)&?)"};

  if (!queryString.empty()) {
    std::string key, value;
    while (RE2::Consume(&queryString, queryParamPattern, &key, &value)) {
      result.queryParams[percentEncode(key)] = percentEncode(value);
    }
  }

  result.scheme = percentEncode(result.scheme);
  result.fragment = percentEncode(result.fragment, '/');

  return result;
}

ParseResult<RobotsTxtRepr> parseRobotsTxt(const std::string& content) {
  RobotsTxtRepr result;
  result["allow"] = nlohmann::json::array();
  result["disallow"] = nlohmann::json::array();

  std::string line;
  std::stringstream ss{content};

  bool inGeneralUserAgent{false};

  while (std::getline(ss, line)) {
    // trim whitespace at both left and right
    line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char c) {
      return !std::isspace(c);
    }));

    line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char c) {
      return !std::isspace(c);
    }).base(), line.end());

    // if the line is empty or starts with a hash (#), it's a comment
    if (line.empty() || line.front() == '#') {
      continue;
    }

    // Parse directive: value pairs
    // Regex captures directive and value (stopping at #), optional trailing comment ignored
    std::string_view lineView{line};
    std::string_view directive, value;
    constexpr char pattern[]{R"(^([A-Za-z-]+)\s*:\s*([^#]*)(?:\s*#.*)?$)"};

    if (!RE2::FullMatch(lineView, pattern, &directive, &value)) {
      continue;
    }

    // trim value
    value.remove_prefix(std::distance(value.begin(),
      std::find_if(value.begin(), value.end(), [](unsigned char c) {
        return !std::isspace(c);
      })));
    value.remove_suffix(std::distance(
      std::find_if(value.rbegin(), value.rend(), [](unsigned char c) {
        return !std::isspace(c);
      }).base(), value.end()));

    std::string directiveStr{directive};
    std::string valueStr{value};

    // Check for User-Agent directive
    if (directiveStr == "User-agent" || directiveStr == "User-Agent") {
      inGeneralUserAgent = (valueStr == "*");
      continue;
    }

    // Only process directives if we're in the general user-agent section
    if (!inGeneralUserAgent) {
      continue;
    }

    if (directiveStr == "Allow" || directiveStr == "allow") {
      if (!valueStr.empty()) {
        result["allow"].push_back(valueStr);
      }
    } else if (directiveStr == "Disallow" || directiveStr == "disallow") {
      if (!valueStr.empty()) {
        result["disallow"].push_back(valueStr);
      }
    } else if (directiveStr == "Crawl-delay" || directiveStr == "crawl-delay") {
      try {
        double delay{std::stod(valueStr)};
        result["crawl-delay"] = delay;
      } catch (std::invalid_argument const&) {
        return std::unexpected(ParseError::ROBOTS_TXT_ERROR);
      } catch (std::out_of_range const&) {
        return std::unexpected(ParseError::ROBOTS_TXT_ERROR);
      }
    }
  }

  return result;
}

bool urlIsDisallowed(const std::string& url, const std::string& pattern) {
  // Empty pattern means nothing is disallowed
  if (pattern.empty()) {
    return false;
  }

  // Extract path from URL (everything after authority, before query/fragment)
  // URL format: scheme://authority/path?query#fragment
  std::string_view urlView{url};

  // Find start of path (after authority)
  std::size_t pathStart{urlView.find("://")};
  if (pathStart == std::string_view::npos) {
    // No scheme, check if it starts with /
    pathStart = 0;
  } else {
    // Skip past "://"
    pathStart += 3;
    // Find the path start (after authority)
    std::size_t authorityEnd{urlView.find('/', pathStart)};
    if (authorityEnd == std::string_view::npos) {
      // No path component
      return false;
    }
    pathStart = authorityEnd;
  }

  // Extract path (stop at ? or #)
  std::size_t pathEnd{urlView.find_first_of("?#", pathStart)};
  std::string path{std::string{urlView.substr(pathStart, pathEnd == std::string_view::npos ? std::string_view::npos : pathEnd - pathStart)}};

  // A pattern of "/" disallows everything
  if (pattern == "/") {
    return true;
  }

  // Check if pattern contains wildcards
  bool hasWildcard{pattern.find('*') != std::string::npos};
  bool hasEndAnchor{!pattern.empty() && pattern.back() == '$'};

  // No wildcards: simple prefix match
  if (!hasWildcard && !hasEndAnchor) {
    if (path.size() < pattern.size()) {
      return false;
    }
    return path.substr(0, pattern.size()) == pattern;
  }

  // Convert robots.txt pattern to regex
  // * matches any sequence of characters (including empty)
  // $ at end means match must reach end of path
  std::string regexPattern{"^"};
  std::string patternToProcess = pattern;

  // Remove trailing $ for processing, but remember it was there
  if (hasEndAnchor) {
    patternToProcess.pop_back();
  }

  for (char c : patternToProcess) {
    if (c == '*') {
      regexPattern += ".*";  // * matches any sequence
    } else if (c == '$') {
      // $ in middle of pattern is treated as literal (rare edge case)
      regexPattern += "\\$";
    } else if (c == '.' || c == '?' || c == '+' || c == '^' || c == '[' ||
               c == ']' || c == '(' || c == ')' || c == '{' || c == '}' ||
               c == '|' || c == '\\') {
      regexPattern += '\\';
      regexPattern += c;
    } else {
      regexPattern += c;
    }
  }

  if (hasEndAnchor) {
    regexPattern += "$";  // Anchor to end of string
  }

  // Match against path
  return RE2::FullMatch(path, regexPattern);
}


}  // namespace url

}  // namespace services

}  // namespace crawler
