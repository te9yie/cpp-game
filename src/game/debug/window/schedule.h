#pragma once
#include "../window.h"

namespace game::debug {

// ScheduleWindow.
class ScheduleWindow : public Window {
 protected:
  virtual const char* menu_item_name() const override { return "Schedule"; }
  virtual void on_render(const DebugGuiApp& ctx) override;
};

}  // namespace game::debug
