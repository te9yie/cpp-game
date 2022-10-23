#include "frame.h"

#include "imgui.h"

namespace sai::core {

void tick_frame(Frame* frame) {
  frame->last_start_count = frame->start_count;
  frame->start_count = SDL_GetPerformanceCounter();
  frame->frequency = SDL_GetPerformanceFrequency();
  ++frame->frame_count;
}

void render_debug_gui(const Frame* frame) {
  auto delta = frame->start_count - frame->last_start_count;
  auto delta_ms = delta * 1000 / frame->frequency;
  ImGui::Text("delta (count): %lu", delta);
  ImGui::Text("delta (ms): %lu", delta_ms);
  ImGui::Text("frame count: %lu", frame->frame_count);
  ImGui::Text("delta: %f", frame->delta());
}

}  // namespace sai::core
