#include "frame.h"

namespace game {

void tick_frame(Frame* frame) {
  frame->last_start_count = frame->start_count;
  frame->start_count = SDL_GetPerformanceCounter();
  ++frame->frame_count;
}

}  // namespace game