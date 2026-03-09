#include "url.h"

#include <array>
#include <cctype>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include "idn2.h"
#include "libpsl.h"
#include "re2/re2.h"

namespace crawler {

namespace services {

namespace url {

constexpr bool isAscii(char c) { return static_cast<unsigned char>(c) <= 127; }

constexpr bool isUnreserved(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~';
}

char hexToDec(std::string_view hexView) {
  if (hexView.size() != 2) {
    throw std::runtime_error("[PERCENT DECODING]: cannot parse hex value");
  }

  static std::array<int, 256> lookup{};
  static std::once_flag lookupInitFlag;

  std::call_once(lookupInitFlag, []() {
    for (auto i{0ULL}; i < 256; ++i) lookup[i] = -1;
    for (char c{'0'}; c <= '9'; ++c)
      lookup[static_cast<unsigned char>(c)] = c - '0';
    for (char c{'A'}; c <= 'F'; ++c)
      lookup[static_cast<unsigned char>(c)] = c - 'A' + 10;
    for (char c{'a'}; c <= 'f'; ++c)
      lookup[static_cast<unsigned char>(c)] = c - 'a' + 10;
  });

  auto high{lookup[static_cast<unsigned char>(hexView[0])]},
      low{lookup[static_cast<unsigned char>(hexView[1])]};

  if (high == -1 || low == -1) {
    throw std::runtime_error("[PERCENT DECODING]: cannot parse hex value");
  }

  return static_cast<char>((high << 4) | low);
}

std::string percentEncode(const std::string& src, char ignore = '\0') {
  std::string result;
  result.reserve(src.size() * 3);

  constexpr char hexStr[]{"0123456789ABCDEF"};

  for (unsigned char c : src) {
    if (isUnreserved(c) || c == ignore) {
      result.push_back(c);
    } else {
      result.push_back('%');
      result.push_back(hexStr[c >> 4]);
      result.push_back(hexStr[c & 0xF]);
    }
  }
  return result;
}

std::string percentDecode(const std::string& src) {
  std::string result;
  result.reserve(src.size());

  for (auto ptr{0ULL}; ptr < src.size();) {
    if (src[ptr] == '%' && ptr + 2 < src.size()) {
      result.push_back(hexToDec(std::string_view(src.c_str() + ptr + 1, 2)));
      ptr += 3;
      continue;
    }
    result.push_back(src[ptr]);
    ++ptr;
  }

  return result;
}

ParseResult parse(const std::string& url) {
  ParseInfo result;

  constexpr char pattern[] =
      R"(^(?:(https?|ftp):\/\/)?([^/?#]+)([^?#]*)(?:\?([^#]*))?(?:#(.*))?$)";

  std::string_view queryString;
  std::string_view authority;

  if (!RE2::FullMatch(url, pattern, &result.scheme, &authority, &result.path,
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
      R"(^(([^@]*)@)?(\[[0-9a-fA-F:.]+\]|[a-zA-Z0-9.-]+)(:(\d+))?$)"};
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
    const char* registeredDomain = psl_registrable_domain(psl, hostStr.c_str());

    if (registeredDomain != nullptr) {
      result.domain = registeredDomain;
      std::string registeredDomainStr(registeredDomain);

      // Extract subdomains: everything before the registered domain
      if (hostStr.length() > registeredDomainStr.length()) {
        size_t domainStart = hostStr.rfind(registeredDomainStr);
        if (domainStart > 0 && hostStr[domainStart - 1] == '.') {
          std::string subdomainPart = hostStr.substr(0, domainStart - 1);

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

  // parse query parameters
  constexpr char queryParamPattern[] = R"(([^=&]+)=([^&]*)&?)";

  if (!queryString.empty()) {
    std::string key, value;
    while (RE2::Consume(&queryString, queryParamPattern, &key, &value)) {
      result.queryParams[percentEncode(key)] = percentEncode(value);
    }
  }

  result.scheme = percentEncode(result.scheme);
  result.path = percentEncode(result.path, '/');
  result.fragment = percentEncode(result.fragment, '/');

  return result;
}

}  // namespace url

}  // namespace services

}  // namespace crawler
