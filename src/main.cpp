#include <SDL.h>

#include <cstdlib>
#include <memory>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "sai/job.h"

namespace {

struct DestroyWindow {
  void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
};
using WindowPtr = std::unique_ptr<SDL_Window, DestroyWindow>;

struct DestroyRenderer {
  void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
};
using RendererPtr = std::unique_ptr<SDL_Renderer, DestroyRenderer>;

}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
  const char* TITLE = "Game";
  const int SCREEN_WIDTH = 16 * 60;
  const int SCREEN_HEIGHT = 9 * 60;

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

  sai::JobExecutor executor("Task");
  executor.start(2);

  Uint64 last_start_time = 0;

  bool loop = true;
  while (loop) {
    auto start_time = SDL_GetPerformanceCounter();

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

    ImGui::ShowDemoWindow();

    ImGui::Begin("Debug");
    {
      if (ImGui::Button("Hello")) {
        executor.submit_func(
            []() { SDL_Log("Hello %u", SDL_GetThreadID(nullptr)); });
      }
      auto delta = start_time - last_start_time;
      auto delta_ms = delta * 1000 / SDL_GetPerformanceFrequency();
      ImGui::Text("delta (count): %lu", delta);
      ImGui::Text("delta (ms): %lu", delta_ms);

      auto canvas_p0 = ImGui::GetCursorScreenPos();
      auto canvas_s = ImGui::GetContentRegionAvail();
      if (canvas_s.x < 50.0f) canvas_s.x = 50.0f;
      if (canvas_s.y < 50.0f) canvas_s.y = 50.0f;
      auto canvas_p1 =
          ImVec2(canvas_p0.x + canvas_s.x, canvas_p0.y + canvas_s.y);

      auto draw = ImGui::GetWindowDrawList();
      draw->AddRectFilled(canvas_p0, canvas_p1,
                          IM_COL32(0x30, 0x30, 0x30, 0xff));
      draw->AddRect(canvas_p0, canvas_p1, IM_COL32(0xff, 0xff, 0xff, 0xff));
    }
    ImGui::End();

    ImGui::Render();

    SDL_SetRenderDrawColor(renderer.get(), 0x12, 0x34, 0x56, 0xff);
    SDL_RenderClear(renderer.get());
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer.get());

    last_start_time = start_time;
  }

  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  return EXIT_SUCCESS;
}
