#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace service {

namespace url {

struct ParseResult {
    std::string scheme;
    std::vector<std::string> subdomains;
    std::string domain;
    std::string port;
    std::string path;
    std::unordered_map<std::string, std::string> queryParams;
    std::string fragment;

    bool parseSuccessful{ false };

    ParseResult() = default;
};

ParseResult parse(const std::string& url);

}

}