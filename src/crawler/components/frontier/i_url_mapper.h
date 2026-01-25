#pragma once
#include <string>

namespace crawler {

namespace component {

class IURLMapper {
public:
    virtual double getPriority(const std::string& url) = 0;
    virtual ~IURLMapper() = default;
};

}

}