#include "video.h"

#include "../debug/gui.h"
#include "../task/app.h"
#include "../task/executor.h"
#include "imgui_impl_sdl.h"

namespace sai::video {

bool init_video_system(VideoSystem* sys, const VideoSettings* settings) {
  VideoSettings default_settings;
  if (!settings) {
    settings = &default_settings;
  }

  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "error: %s", SDL_GetError());
    return false;
  }
  sys->shutdown.func = []() { SDL_QuitSubSystem(SDL_INIT_VIDEO); };

  WindowPtr window(SDL_CreateWindow(
      settings->title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      settings->screen_width, settings->screen_height, settings->window_flags));
  if (!window) {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "error: %s", SDL_GetError());
    return false;
  }

  RendererPtr renderer(
      SDL_CreateRenderer(window.get(), -1, settings->renderer_flags));
  if (!renderer) {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "error: %s", SDL_GetError());
    return false;
  }

  sys->window = std::move(window);
  sys->renderer = std::move(renderer);

  return true;
}

void handle_events(VideoSystem* sys, RenderSize* size,
                   task::EventWriter<WindowEvent> writer, debug::Gui*) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    ImGui_ImplSDL2_ProcessEvent(&e);
    if (e.type == SDL_QUIT) {
      writer.notify(WindowEvent::Quit);
    }
    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE &&
        e.window.windowID == SDL_GetWindowID(sys->window.get())) {
      writer.notify(WindowEvent::Quit);
    }
  }
  auto r = sys->renderer.get();
  SDL_GetRendererOutputSize(r, &size->w, &size->h);
}

void begin_render(VideoSystem* sys) {
  auto r = sys->renderer.get();
  SDL_SetRenderDrawColor(r, 0x12, 0x34, 0x56, 0xff);
  SDL_RenderClear(r);
}

void end_render(VideoSystem* sys) { SDL_RenderPresent(sys->renderer.get()); }

void preset_video(task::App* app) {
  app->add_context<VideoSystem>();
  app->add_context<RenderSize>();

  app->add_event<WindowEvent>();

  app->add_setup_task(init_video_system);
  app->add_task_in_phase<task::FirstPhase>(
      "handle events", handle_events,
      task::TaskOption().exclusive_this_thread());
  app->add_task_in_phase<task::PreRenderPhase>("== begin render", begin_render);
  app->add_task_in_phase<task::LastPhase>(
      "== end render", end_render, task::TaskOption().exclusive_this_thread());
}

}  // namespace sai::video
