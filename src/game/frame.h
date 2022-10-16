#pragma once
#include "config.h"

namespace game {

struct Frame {
  Uint64 last_start_count = 0;
  Uint64 start_count = 0;
  Uint64 frame_count = 0;
};

void tick_frame(Frame* frame);

}  // namespace game