#pragma once

#include "components/frontier/frontier_builder.h"
#include "components/frontier/round_robin/round_robin.h"
#include "components/frontier/sample/sample_implements.h"
#include "components/worker/worker_pool.h"

namespace crawler {

using namespace components;

class Spider {
 public:
  void run() const;

 private:
  Frontier frontier;
};

};  // namespace crawler