#include "url.h"
#include "services/url.h"

#include <stdexcept>    

using namespace services::url;

namespace crawler {

namespace types {

URL::URL(std::string&& url) 
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
    } else {
        // Parsing failed, throw exception (caller must handle it)
        throw std::invalid_argument("Invalid URL: " + url_);
    }
}

URL::URL(const std::string& url) 
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
    else {
        // Parsing failed, throw exception (caller must handle it)
        throw std::invalid_argument("Invalid URL: " + url_);
    }
}

}

}

