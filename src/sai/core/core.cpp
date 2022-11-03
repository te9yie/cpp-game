#include "core.h"

#include <SDL.h>

namespace sai::core {

bool init_core(Core* c) {
  if (SDL_Init(SDL_INIT_TIMER) < 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "error: %s", SDL_GetError());
    return false;
  }
  c->shutdown.func = []() { SDL_Quit(); };
  return true;
}

}  // namespace sai::core
