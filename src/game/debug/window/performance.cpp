#include "performance.h"

#include "sai/debug/performance.h"

namespace game::debug {

/*virtual*/ void PerformanceWindow::on_render(
    const DebugGuiApp& app) /*override*/ {
  auto profiler = app.get_context<sai::debug::PerformanceProfiler>();
  profiler->render_debug_gui();
}

}  // namespace game::debug
