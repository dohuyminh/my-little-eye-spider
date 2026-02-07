#pragma once

#include "components/frontier/sample/sample_implements.h"
#include "components/frontier/round_robin/round_robin.h"
#include "components/frontier/frontier_builder.h"

namespace crawler {

using namespace components;

class Spider {
public:

    
    void run() const;

private:

    Frontier frontier;

};

};