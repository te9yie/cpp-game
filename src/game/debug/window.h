#pragma once
#include "local_app.h"

namespace game::debug {

// Window.
class Window {
 private:
  bool is_open_ = false;

 public:
  virtual ~Window() = default;

  void show_menu_item();
  void show_window(const DebugGuiApp& app);

 protected:
  virtual const char* menu_item_name() const = 0;
  virtual const char* window_name() const { return menu_item_name(); }
  virtual void on_render(const DebugGuiApp& app) = 0;
};

}  // namespace game::debug
