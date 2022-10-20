#include "video.h"

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
  sys->shutdown_system = []() {
    // clang-format off
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    // clang-format on
  };

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

void begin_render(VideoSystem* sys) {
  SDL_SetRenderDrawColor(sys->renderer.get(), 0x12, 0x34, 0x56, 0xff);
  SDL_RenderClear(sys->renderer.get());
}

void end_render(VideoSystem* sys) { SDL_RenderPresent(sys->renderer.get()); }

}  // namespace sai::video
