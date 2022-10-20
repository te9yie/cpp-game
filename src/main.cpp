#include <SDL.h>

#include <cstdlib>
#include <memory>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "sai/task/executor.h"

namespace {

struct DestroyWindow {
  void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
};
using WindowPtr = std::unique_ptr<SDL_Window, DestroyWindow>;

struct DestroyRenderer {
  void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
};
using RendererPtr = std::unique_ptr<SDL_Renderer, DestroyRenderer>;

void update_int(int* i) {
  auto pre_i = *i;
  *i += 10;
  SDL_Log("update: %d -> %d", pre_i, *i);
}
void read_int(const int* i) { SDL_Log("read: %d", *i); }

}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
  const char* TITLE = "Game";
  const int SCREEN_WIDTH = 16 * 60;
  const int SCREEN_HEIGHT = 9 * 60;

  sai::task::Context context;
  auto tasks = context.add<sai::task::Executor>("TaskExecutor");
  tasks->setup(2);

  context.add<int>(10);

  tasks->add_task("1 update int", update_int);
  tasks->add_task("2 read int", read_int);
  tasks->add_task("3 read int", read_int);
  tasks->add_task("4 update int", update_int);
  tasks->add_task("5 read int", read_int);
  tasks->add_task("6 read int", read_int);
  tasks->add_task("7 update int", update_int);

  tasks->run(&context);

  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }
  atexit(SDL_Quit);

  WindowPtr window(SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                    SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE));
  if (!window) {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  RendererPtr renderer(SDL_CreateRenderer(
      window.get(), -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));
  if (!renderer) {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplSDL2_InitForSDLRenderer(window.get(), renderer.get());
  ImGui_ImplSDLRenderer_Init(renderer.get());

  bool loop = true;
  while (loop) {
    {
      SDL_Event e;
      while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        if (e.type == SDL_QUIT) {
          loop = false;
        }
        if (e.type == SDL_WINDOWEVENT &&
            e.window.event == SDL_WINDOWEVENT_CLOSE &&
            e.window.windowID == SDL_GetWindowID(window.get())) {
          loop = false;
        }
      }
    }

    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();
    ImGui::Begin("Debug");
    tasks->render_debug_gui();
    ImGui::End();

    ImGui::Render();

    SDL_SetRenderDrawColor(renderer.get(), 0x12, 0x34, 0x56, 0xff);
    SDL_RenderClear(renderer.get());

    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(renderer.get());
  }

  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  return EXIT_SUCCESS;
}
