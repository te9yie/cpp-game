#include "gui.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

namespace sai::debug {

bool init_gui_system(Gui* gui, const video::VideoSystem* video) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplSDL2_InitForSDLRenderer(video->window.get(), video->renderer.get());
  ImGui_ImplSDLRenderer_Init(video->renderer.get());

  gui->shutdown_system = []() {
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  };

  return true;
}

void begin_frame(Gui*) {
  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}
void end_frame(Gui*) { ImGui::Render(); }
void render_gui(Gui*) {
  ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
}

}  // namespace sai::debug