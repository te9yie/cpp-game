#pragma once
#include "../window.h"

namespace game::debug {

// PerformanceWindow.
class PerformanceWindow : public Window {
 protected:
  virtual const char* menu_item_name() const override { return "Performance"; }
  virtual void on_render(const DebugGuiApp& ctx) override;
};

}  // namespace game::debug
