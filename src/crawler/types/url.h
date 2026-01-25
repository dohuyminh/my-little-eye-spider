#pragma once

#include <string>

namespace crawler {

class URL {
public:

    URL() = default;

    URL(std::string&& url, double priority) noexcept;
    
    URL(const std::string& url, double priority) noexcept;

    const std::string& url() const noexcept {
        return url_; 
    }

    double priority() const noexcept {
        return priority_;
    }

    std::string to_string() const noexcept {
        return url_;
    }

    auto friend operator<=>(const URL& u1, const URL& u2) noexcept {
        return u1.priority_ <=> u2.priority_;
    }

private:
    std::string url_;
    double priority_;
};

}