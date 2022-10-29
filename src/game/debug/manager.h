#pragma once
#include <deque>
#include <memory>

#include "context.h"
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
  void show_menu(const DebugGuiContext& ctx);
  void show_window(const DebugGuiContext& ctx);
};

void preset_debug(sai::task::App* app);
bool setup_debug_gui(sai::debug::Gui*, Manager* mgr);
void render_debug_gui(sai::debug::Gui*, Manager* mgr,
                      const DebugGuiContext& ctx);

}  // namespace game::debug
