#include "url.h"

namespace crawler {

URL::URL(std::string&& url) noexcept
    : url_(std::move(url))
{
    // Parsing logic to initialize scheme_, authority_, path_, queryParams_, fragment_
    // would go here.
}

URL::URL(const std::string& url) noexcept
    : url_(url)
{
    // Parsing logic to initialize scheme_, authority_, path_, queryParams_, fragment_
    // would go here.
}

}

