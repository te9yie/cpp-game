#include "core.h"

#include <SDL.h>

#include "imgui_impl_sdl.h"
#include "sai/task/app.h"

namespace sai::core {

bool init_system(System* sys) {
  if (SDL_Init(SDL_INIT_TIMER) < 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "error: %s", SDL_GetError());
    return false;
  }
  sys->shutdown_system = []() { SDL_Quit(); };
  return true;
}

void handle_events(task::ExecutorWork* work, video::VideoSystem* sys) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    ImGui_ImplSDL2_ProcessEvent(&e);
    if (e.type == SDL_QUIT) {
      work->loop = false;
    }
    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE &&
        e.window.windowID == SDL_GetWindowID(sys->window.get())) {
      work->loop = false;
    }
  }
}

void preset_core(task::App* app) {
  app->add_context<System>();
  app->add_setup_task(init_system);
}

}  // namespace sai::core
