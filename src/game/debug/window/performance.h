#pragma once
#include "../window.h"

namespace game::debug {

// PerformanceWindow.
class PerformanceWindow : public Window {
 protected:
  virtual const char* menu_item_name() const override { return "Performance"; }
  virtual void on_render(const DebugGuiApp& app) override;
};

}  // namespace game::debug
