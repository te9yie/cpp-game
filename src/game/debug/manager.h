#pragma once
#include <deque>
#include <memory>

#include "local_app.h"
#include "sai/debug/gui.h"
#include "sai/task/fwd.h"
#include "window.h"

namespace game::debug {

// Manager.
class Manager {
 private:
  std::deque<std::unique_ptr<Window>> windows_;

 public:
  bool setup();
  void show_menu(const DebugGuiApp& app);
  void show_window(const DebugGuiApp& app);
};

void preset_debug(sai::task::App* app);
bool setup_debug_gui(sai::debug::Gui*, Manager* mgr);
void render_debug_gui(sai::debug::Gui*, Manager* mgr, const DebugGuiApp& app);

}  // namespace game::debug
