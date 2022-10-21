#pragma once
#include <SDL.h>

#include <memory>
#include <string>

#include "t9/defer.h"

namespace sai::video {

// WindowPtr.
struct DestroyWindow {
  void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
};
using WindowPtr = std::unique_ptr<SDL_Window, DestroyWindow>;

// RendererPtr.
struct DestroyRenderer {
  void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
};
using RendererPtr = std::unique_ptr<SDL_Renderer, DestroyRenderer>;

// VideoSystem.
struct VideoSystem {
  t9::Defer shutdown_system;
  WindowPtr window;
  RendererPtr renderer;
};

// VideoSettings.
struct VideoSettings {
  std::string title;
  int screen_width = 16 * 50;
  int screen_height = 9 * 50;
  Uint32 window_flags = SDL_WINDOW_RESIZABLE;
  Uint32 renderer_flags = SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED;
};

bool init_video_system(VideoSystem* sys, const VideoSettings* settings);
void begin_render(VideoSystem* sys);
void end_render(VideoSystem* sys);

}  // namespace sai::video
