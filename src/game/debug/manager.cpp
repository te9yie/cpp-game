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
void Manager::show_menu(const DebugGuiApp&) {
  std::for_each(windows_.begin(), windows_.end(),
                [](auto& window) { window->show_menu_item(); });
}
void Manager::show_window(const DebugGuiApp& app) {
  std::for_each(windows_.begin(), windows_.end(),
                [app](auto& window) { window->show_window(app); });
}

void preset_debug(sai::task::App* app) {
  app->add_context<Manager>();
  app->add_setup_task(setup_debug_gui);
  app->add_task("render debug gui", render_debug_gui);
}

bool setup_debug_gui(sai::debug::Gui*, Manager* mgr) { return mgr->setup(); }

void render_debug_gui(sai::debug::Gui*, Manager* mgr, const DebugGuiApp& app) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Game")) {
      mgr->show_menu(app);
      ImGui::Separator();
      if (ImGui::MenuItem("Quit")) {
        auto writer = app.get_event_writer<sai::video::WindowEvent>();
        writer.notify(sai::video::WindowEvent::Quit);
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  mgr->show_window(app);
}

}  // namespace game::debug
