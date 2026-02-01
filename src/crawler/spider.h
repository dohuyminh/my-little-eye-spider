#pragma once

#include "components/frontier/sample/sample_implements.h"
#include "components/frontier/frontier_builder.h"
#include "components/frontier/frontier.h"

namespace crawler {

using namespace component;

class Spider {
public:

    
    void run() const;

private:

    Frontier frontier;

};

};