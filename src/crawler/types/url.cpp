#include "url.h"
#include "services/url.h"

using namespace services::url;

namespace crawler {

namespace types {

URL::URL(std::string&& url) noexcept
    : url_(std::move(url))
{
    ParseResult result = parse(url_);
    
    if (result.parseSuccessful) {
        scheme_ = std::move(result.scheme);
        subdomains_ = std::move(result.subdomains);
        domain_ = std::move(result.domain);
        port_ = std::move(result.port);
        path_ = std::move(result.path);
        queryParams_ = std::move(result.queryParams);
        fragment_ = std::move(result.fragment);
    }
}

URL::URL(const std::string& url) noexcept
    : url_(url)
{
    ParseResult result = parse(url_);
    
    if (result.parseSuccessful) {
        scheme_ = std::move(result.scheme);
        subdomains_ = std::move(result.subdomains);
        domain_ = std::move(result.domain);
        port_ = std::move(result.port);
        path_ = std::move(result.path);
        queryParams_ = std::move(result.queryParams);
        fragment_ = std::move(result.fragment);
    }
}

}

}

