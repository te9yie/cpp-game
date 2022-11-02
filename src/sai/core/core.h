#pragma once
#include "../task/fwd.h"
#include "t9/defer.h"

namespace sai::core {

// Core.
struct Core {
  t9::Defer shutdown;
};

// init_core
bool init_core(Core* c);
void preset_core(task::App* app);

}  // namespace sai::core