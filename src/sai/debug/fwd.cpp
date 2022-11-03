#include "fwd.h"

#include "../task/app.h"
#include "../video/video.h"
#include "gui.h"
#include "performance.h"

namespace sai::debug {

void preset_debug_gui(task::App* app) {
  app->add_context<PerformanceProfiler>();
  app->add_context<Gui>();

  app->add_setup_task(init_debug_gui);
  app->add_setup_task([](sai::debug::PerformanceProfiler* profiler) {
    profiler->setup_thread("MainThread");
    return true;
  });

  app->add_task_in_phase<task::FirstPhase>(
      "performance tick",
      [](sai::debug::PerformanceProfiler* profiler) { profiler->tick(); });
  app->add_task_in_phase<task::PreUpdatePhase>("-- begin debug gui",
                                               begin_debug_gui);
  app->add_task_in_phase<task::PostUpdatePhase>("-- end debug gui",
                                                end_debug_gui);
  app->add_task_in_phase<task::PostRenderPhase>("render debug gui",
                                                render_debug_gui);
}

}  // namespace sai::debug
