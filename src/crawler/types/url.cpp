#include "url.h"

namespace crawler {

URL::URL(std::string&& url, double priority) noexcept :
    url_(std::move(url)), priority_(priority) {}

URL::URL(const std::string& url, double priority) noexcept :
    url_(url), priority_(priority) {}

}

