#pragma once

#include <string>
#include <unordered_map>

namespace crawler {

class URL {
public:

    URL() = default;

    URL(std::string&& url) noexcept;
    
    URL(const std::string& url) noexcept;

    const std::string& url() const noexcept {
        return url_; 
    }

    // std::string scheme() const noexcept {
    //     return std::string(scheme_);
    // }

    // std::string authority() const noexcept {
    //     return std::string(authority_);
    // }

    // std::string path() const noexcept {
    //     return std::string(path_);
    // }

    // std::optional<std::string> query(const std::string& key) const noexcept {
    //     auto it = queryParams_.find(key);
    //     if (it != queryParams_.end()) {
    //         return it->second;
    //     }
    //     return std::nullopt;
    // }

    // std::string fragment() const noexcept {
    //     return std::string(fragment_);
    // }

    std::string to_string() const noexcept {
        return url_;
    }

private:
    std::string url_;

    // std::string_view scheme_;
    // std::string_view authority_;
    // std::string_view path_;
    // std::unordered_map<std::string, std::string> queryParams_;
    // std::string_view fragment_;

};

}