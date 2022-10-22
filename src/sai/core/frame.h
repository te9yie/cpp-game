#pragma once
#include <SDL.h>

namespace sai::core {

// Frame.
struct Frame {
  Uint64 last_start_count = 0;
  Uint64 start_count = 0;
  Uint64 frequency = 0;
  Uint64 frame_count = 0;

  float delta() const {
    return static_cast<float>(start_count - last_start_count) / frequency;
  }
};

void tick_frame(Frame* frame);

}  // namespace sai::core
