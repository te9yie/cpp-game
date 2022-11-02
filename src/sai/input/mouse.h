#pragma once
#include <SDL.h>

#include "../task/fwd.h"

namespace sai::input {

// MouseState.
struct MouseState {
  int x;
  int y;
  Uint32 buttons = 0;
};

void update_mouse(MouseState* state);

void preset_input(task::App* app);

}  // namespace sai::input