#include "frame.h"

namespace sai::core {

void tick_frame(Frame* frame) {
  frame->last_start_count = frame->start_count;
  frame->start_count = SDL_GetPerformanceCounter();
  frame->frequency = SDL_GetPerformanceFrequency();
  ++frame->frame_count;
}

}  // namespace sai::core
