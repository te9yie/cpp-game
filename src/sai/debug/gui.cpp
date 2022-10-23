#include "gui.h"

#include "../task/app.h"
#include "../video/video.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "performance.h"

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

void render_gui(video::VideoSystem*, Gui*) {
  ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
}

void preset_debug_gui(task::App* app) {
  app->add_context<PerformanceProfiler>();
  app->add_context<Gui>();

  app->add_setup_task(init_gui_system);
  app->add_setup_task([](sai::debug::PerformanceProfiler* profiler) {
    profiler->setup_thread("MainThread");
    return true;
  });

  app->add_task_in_phase<task::FirstPhase>(
      "performance tick",
      [](sai::debug::PerformanceProfiler* profiler) { profiler->tick(); });
  app->add_task_in_phase<task::PreUpdatePhase>("-- begin debug gui",
                                               begin_frame);
  app->add_task_in_phase<task::PostUpdatePhase>("-- end debug gui", end_frame);
  app->add_task_in_phase<task::RenderPhase>("render debug gui", render_gui);
}

}  // namespace sai::debug