#include "mouse.h"

#include "../task/app.h"

namespace sai::input {

void update_mouse(MouseState* state) {
  state->buttons = SDL_GetMouseState(&state->x, &state->y);
}

void preset_input(task::App* app) {
  app->add_context<MouseState>();
  app->add_task_in_phase<task::PreUpdatePhase>("update mouse", update_mouse);
}

}  // namespace sai::input