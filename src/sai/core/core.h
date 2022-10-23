#pragma once
#include "../task/fwd.h"
#include "t9/defer.h"

namespace sai::core {

// System.
struct System {
  t9::Defer shutdown_system;
};

bool init_system(System* sys);
void preset_core(task::App* app);

}  // namespace sai::core