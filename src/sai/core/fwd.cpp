#include "fwd.h"

#include "../task/app.h"
#include "../task/phase.h"
#include "core.h"
#include "frame.h"

namespace sai::core {

void preset_core(task::App* app) {
  app->add_context<Core>();
  app->add_context<Frame>();
  app->add_setup_task(init_core);
  app->add_task_in_phase<task::FirstPhase>("tick frame", tick_frame);
}

}  // namespace sai::core