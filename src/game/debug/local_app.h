#pragma once
#include "sai/core/frame.h"
#include "sai/debug/performance.h"
#include "sai/ecs/registry.h"
#include "sai/task/local_app.h"
#include "sai/task/scheduler.h"

namespace game::debug {

// DebugGuiApp.
using DebugGuiApp =
    sai::task::LocalApp<sai::core::Frame*, sai::task::Scheduler*,
                        sai::ecs::Registry*, sai::debug::PerformanceProfiler*>;

}  // namespace game::debug
