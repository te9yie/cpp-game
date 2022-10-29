#include "schedule.h"

#include "sai/task/scheduler.h"

namespace game::debug {

/*virtual*/ void ScheduleWindow::on_render(
    const DebugGuiApp& app) /*override*/ {
  auto scheduler = app.get_context<sai::task::Scheduler>();
  scheduler->render_debug_gui();
}

}  // namespace game::debug
