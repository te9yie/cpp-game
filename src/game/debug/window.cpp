#include "window.h"

namespace game::debug {

void Window::show_menu_item() {
  ImGui::MenuItem(menu_item_name(), nullptr, &is_open_);
}

void Window::show_window(const DebugGuiApp& app) {
  if (!is_open_) return;

  if (ImGui::Begin(window_name(), &is_open_)) {
    on_render(app);
  }
  ImGui::End();
}

}  // namespace game::debug
