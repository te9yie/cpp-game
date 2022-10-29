#pragma once
#include "sai/core/frame.h"
#include "sai/debug/performance.h"
#include "sai/ecs/registry.h"
#include "sai/task/context_ref.h"
#include "sai/task/scheduler.h"

namespace game::debug {

// DebugGuiContext.
using DebugGuiContext =
    sai::task::ContextRef<sai::core::Frame*, sai::task::Scheduler*,
                          sai::ecs::Registry*,
                          sai::debug::PerformanceProfiler*>;

}  // namespace game::debug
