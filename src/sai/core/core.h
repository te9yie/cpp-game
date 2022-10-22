#pragma once
#include "sai/task/executor.h"
#include "sai/video/video.h"
#include "t9/defer.h"

namespace sai::task {
class App;
}

namespace sai::core {

// System.
struct System {
  t9::Defer shutdown_system;
};

bool init_system(System* sys);
void handle_events(task::ExecutorWork* work, video::VideoSystem* sys);

void preset_core(task::App* app);

}  // namespace sai::core