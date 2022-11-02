#include "core.h"

#include <SDL.h>

#include "../task/app.h"
#include "../task/phase.h"
#include "frame.h"

namespace sai::core {

bool init_core(Core* c) {
  if (SDL_Init(SDL_INIT_TIMER) < 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "error: %s", SDL_GetError());
    return false;
  }
  c->shutdown.func = []() { SDL_Quit(); };
  return true;
}

void preset_core(task::App* app) {
  app->add_context<Core>();
  app->add_context<Frame>();
  app->add_setup_task(init_core);
  app->add_task_in_phase<task::FirstPhase>("tick frame", tick_frame);
}

}  // namespace sai::core
