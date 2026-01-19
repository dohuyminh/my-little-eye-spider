#pragma once

#include "types/url.h"
#include "i_url_mapper.h"

#include <memory>
#include <utility>

namespace crawler {

namespace component {

class IFrontPrioritizer {
public:
    virtual std::pair<URL, std::size_t> selectQueue(const std::string& url) = 0;
    virtual ~IFrontPrioritizer() = default;
};

}

}