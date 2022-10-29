#include "window.h"

namespace game::debug {

void Window::show_menu_item() {
  ImGui::MenuItem(menu_item_name(), nullptr, &is_open_);
}

void Window::show_window(const DebugGuiApp& ctx) {
  if (!is_open_) return;

  if (ImGui::Begin(window_name(), &is_open_)) {
    on_render(ctx);
  }
  ImGui::End();
}

}  // namespace game::debug
