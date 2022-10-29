#include "performance.h"

#include "../context.h"
#include "sai/debug/performance.h"

namespace game::debug {

/*virtual*/ void PerformanceWindow::on_render(
    const DebugGuiContext& ctx) /*override*/ {
  auto profiler = ctx.get_context<sai::debug::PerformanceProfiler>();
  profiler->render_debug_gui();
}

}  // namespace game::debug
