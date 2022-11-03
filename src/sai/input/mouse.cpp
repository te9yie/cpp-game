#include "mouse.h"

namespace sai::input {

void update_mouse(MouseState* state) {
  state->buttons = SDL_GetMouseState(&state->x, &state->y);
}

}  // namespace sai::input