#include "fwd.h"

#include "../task/app.h"
#include "asset.h"
#include "manager.h"

namespace sai::asset {

void preset_asset(task::App* app) {
  auto ev = app->add_event<AssetEvent>();
  auto assets = app->add_context<AssetStorage>(ev);
  app->add_context<Manager>(assets);
  app->add_setup_task(init_manager);
  app->add_task_in_phase<task::FirstPhase>("update asset manager",
                                           update_manager);
}

}  // namespace sai::asset