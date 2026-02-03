#include "url.h"

#include "re2/re2.h"
#include "idn2.h"
#include "libpsl.h"

namespace services {

namespace url {

constexpr bool isAscii(char c) {
    return static_cast<unsigned char>(c) <= 127;
}


ParseResult parse(const std::string& url) {
    ParseResult result;

    static const re2::RE2 pattern(R"(^(?:(https?):\/\/)?([^/?#]+)([^?#]*)(?:\?([^#]*))?(?:#(.*))?$)");
    
    std::string_view queryString;
    std::string_view authority;
    
    if (!RE2::FullMatch(url, pattern,
                      &result.scheme,
                      &authority,
                      &result.path,
                      &queryString,
                      &result.fragment)) {
        // Handle parsing error if needed
        return result;
    }

    // validate authority 
    if (authority.empty()) {
        return result;
    }

    // normalize authority (handle IDN conversion and port extraction)
    std::string normalizedAuthority(authority);
    std::string portStr;
    
    // Extract port first (port is at the end, after last ':')
    size_t colonPos = normalizedAuthority.rfind(':');
    if (colonPos != std::string::npos) {
        // Verify what follows ':' is actually a port (digits only)
        bool isPort = std::all_of(normalizedAuthority.begin() + colonPos + 1, 
                                  normalizedAuthority.end(), 
                                  [](char c) { return std::isdigit(c); });
        if (isPort) {
            portStr = normalizedAuthority.substr(colonPos + 1);
            result.port = portStr;
            normalizedAuthority = normalizedAuthority.substr(0, colonPos);
        }
    }
    
    // Check if host part contains non-ASCII (IDN domain)
    bool hasNonAscii = std::any_of(normalizedAuthority.begin(), normalizedAuthority.end(), 
        [](char c) { return !isAscii(c); });

    // Convert IDN to ASCII (punycode) if needed
    if (hasNonAscii) {
        char* asciiOutput = nullptr;
        int rc = idn2_to_ascii_8z(normalizedAuthority.c_str(), &asciiOutput, 0);
        if (rc == IDN2_OK && asciiOutput != nullptr) {
            normalizedAuthority = std::string(asciiOutput);
            idn2_free(asciiOutput);
        } else {
            // IDN conversion failed, use original
            return result;
        }
    }

    // Validate authority format (regex) - now on normalized authority
    static const re2::RE2 authorityPattern(R"(^[a-zA-Z0-9:._\-]+$)");
    if (!RE2::FullMatch(normalizedAuthority, authorityPattern)) {
        return result;  // Invalid authority format
    }
    
    // Split authority into labels by '.'
    std::vector<std::string> labels;
    std::string label;
    for (char c : normalizedAuthority) {
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
    const psl_ctx_t* psl = psl_builtin();
    if (psl != nullptr) {
        const char* registeredDomain = psl_registrable_domain(psl, normalizedAuthority.c_str());
        
        if (registeredDomain != nullptr) {
            result.domain = registeredDomain;
            std::string registeredDomainStr(registeredDomain);
            
            // Extract subdomains: everything before the registered domain
            if (normalizedAuthority.length() > registeredDomainStr.length()) {
                size_t domainStart = normalizedAuthority.rfind(registeredDomainStr);
                if (domainStart > 0 && normalizedAuthority[domainStart - 1] == '.') {
                    std::string subdomainPart = normalizedAuthority.substr(0, domainStart - 1);
                    
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
                result.domain = labels[labels.size() - 2] + "." + labels[labels.size() - 1];
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
            result.domain = labels[labels.size() - 2] + "." + labels[labels.size() - 1];
            for (size_t i = 0; i < labels.size() - 2; ++i) {
                result.subdomains.push_back(labels[i]);
            }
        } else if (labels.size() == 1) {
            result.domain = labels[0];
        }
    }
    
    // parse query parameters
    static const re2::RE2 queryParamPattern(R"(([^=&]+)=([^&]*)&?)");
    
    if (!queryString.empty()) {
        std::string key, value;
        while (RE2::Consume(&queryString, queryParamPattern, &key, &value)) {
            result.queryParams[key] = value;
        }
    }

    result.parseSuccessful = true;

    return result;
}

}

}