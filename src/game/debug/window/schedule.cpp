#include "schedule.h"

#include "sai/task/scheduler.h"

namespace game::debug {

/*virtual*/ void ScheduleWindow::on_render(
    const DebugGuiApp& ctx) /*override*/ {
  auto scheduler = ctx.get_context<sai::task::Scheduler>();
  scheduler->render_debug_gui();
}

}  // namespace game::debug