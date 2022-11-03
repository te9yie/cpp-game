#pragma once
#include "../video/fwd.h"
#include "t9/defer.h"

namespace sai::debug {

// Gui.
struct Gui {
  t9::Defer shutdown;
};

bool init_debug_gui(Gui* gui, const video::VideoSystem* video);
void begin_debug_gui(Gui*);
void end_debug_gui(Gui*);
void render_debug_gui(video::VideoSystem* video, Gui*);

}  // namespace sai::debug
