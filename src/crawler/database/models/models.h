#pragma once

#include <memory>
#include "database/models/metadata.h"
#include "url.h"
#include "url_contents.h"
#include "url_metadata.h"
#include "url_outlinks.h"
#include "robots_txt.h"

namespace crawler::database {

using Model = std::shared_ptr<PostgresModel>;

}
