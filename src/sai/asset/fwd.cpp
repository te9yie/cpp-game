#include "fwd.h"

#include "../task/app.h"
#include "manager.h"

namespace sai::asset {

void preset_asset(task::App* app) {
  app->add_context<Manager>();
  app->add_task_in_phase<task::FirstPhase>("update asset manager",
                                           update_manager);
}

}  // namespace sai::asset