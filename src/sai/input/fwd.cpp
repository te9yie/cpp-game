#include "fwd.h"

#include "../task/app.h"
#include "mouse.h"

namespace sai::input {

void preset_input(task::App* app) {
  app->add_context<MouseState>();
  app->add_task_in_phase<task::PreUpdatePhase>("update mouse", update_mouse);
}

}  // namespace sai::input