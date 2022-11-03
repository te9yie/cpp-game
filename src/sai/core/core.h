#pragma once
#include "t9/defer.h"

namespace sai::core {

// Core.
struct Core {
  t9::Defer shutdown;
};

// init_core
bool init_core(Core* c);

}  // namespace sai::core