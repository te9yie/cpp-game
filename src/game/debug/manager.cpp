#include "manager.h"

#include "sai/task/app.h"
#include "window/performance.h"
#include "window/schedule.h"

namespace game::debug {

bool Manager::setup() {
  windows_.emplace_back(std::make_unique<PerformanceWindow>());
  windows_.emplace_back(std::make_unique<ScheduleWindow>());
  return true;
}
void Manager::show_menu(const DebugGuiApp& /*ctx*/) {
  std::for_each(windows_.begin(), windows_.end(),
                [](auto& window) { window->show_menu_item(); });
}
void Manager::show_window(const DebugGuiApp& ctx) {
  std::for_each(windows_.begin(), windows_.end(),
                [ctx](auto& window) { window->show_window(ctx); });
}

void preset_debug(sai::task::App* app) {
  app->add_context<Manager>();
  app->add_setup_task(setup_debug_gui);
  app->add_task("render debug gui", render_debug_gui);
}

bool setup_debug_gui(sai::debug::Gui*, Manager* mgr) { return mgr->setup(); }

void render_debug_gui(sai::debug::Gui*, Manager* mgr, const DebugGuiApp& ctx) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Game")) {
      mgr->show_menu(ctx);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  mgr->show_window(ctx);
}

}  // namespace game::debug
