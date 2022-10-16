#include <SDL.h>

#include <cstdlib>
#include <memory>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "sai/performance_profiler.h"
#include "sai/task.h"
#include "sai/task_arg_ex.h"

namespace {

struct Frame {
  Uint64 last_start_count = 0;
  Uint64 start_count = 0;
  Uint64 frame_count = 0;
};

void tick_frame(Frame* frame) {
  frame->last_start_count = frame->start_count;
  frame->start_count = SDL_GetPerformanceCounter();
  ++frame->frame_count;
}

struct System {
  sai::PerformanceProfiler* profiler = nullptr;
  sai::TaskExecutor* tasks = nullptr;
};

// ImGui にマルチスレッドでアクセスしないための識別子
// 引数に `MutexRes<DebugGui>` をもつ関数は同時に実行されない。
struct DebugGui {};

void draw_frame_info(const Frame* frame, const System* sys,
                     sai::MutexRes<DebugGui>) {
  ImGui::Begin("Debug");
  {
    auto delta = frame->start_count - frame->last_start_count;
    auto delta_ms = delta * 1000 / SDL_GetPerformanceFrequency();
    ImGui::Text("delta (count): %lu", delta);
    ImGui::Text("delta (ms): %lu", delta_ms);
    ImGui::Text("frame count: %lu", frame->frame_count);

    ImGui::Separator();
    ImGui::Text("<Tasks>");
    sys->tasks->render_debug_gui();

    ImGui::Separator();
    ImGui::Text("<Profiler>");
    sys->profiler->render_debug_gui();
  }
  ImGui::End();
}

void setup_task(sai::TaskExecutor* tasks) {
  {  // setup context.
    tasks->add_context<Frame>();
    tasks->add_context<DebugGui>();
  }

  {  // setup task.
    auto fence = sai::TaskOption().set_fence();

    tasks->add_task("tick frame", tick_frame, fence);
    tasks->add_task("show demo",
                    [](sai::MutexRes<DebugGui>) { ImGui::ShowDemoWindow(); });
    tasks->add_task("draw frame info", draw_frame_info);
  }
}

}  // namespace

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

  sai::PerformanceProfiler profiler;
  sai::TaskExecutor tasks("TaskExecutor");
  if (!tasks.setup(2)) return false;

  tasks.add_context<System>(System{&profiler, &tasks});

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

  setup_task(&tasks);

  bool loop = true;
  while (loop) {
    profiler.tick();
    {
      PERF_TAG("HandleEvents");
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
    {
      PERF_TAG("Update");

      ImGui_ImplSDLRenderer_NewFrame();
      ImGui_ImplSDL2_NewFrame();
      ImGui::NewFrame();

      tasks.run();

      ImGui::Render();
    }
    {
      PERF_TAG("Render");
      SDL_SetRenderDrawColor(renderer.get(), 0x12, 0x34, 0x56, 0xff);
      SDL_RenderClear(renderer.get());
      ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    }
    {
      PERF_TAG("Swap");
      SDL_RenderPresent(renderer.get());
    }
  }

  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  return EXIT_SUCCESS;
}
