#pragma once
#include "sai/video/video.h"
#include "t9/defer.h"

namespace sai::debug {

// Gui.
struct Gui {
  t9::Defer shutdown_system;
};

bool init_gui_system(Gui* gui, const video::VideoSystem* video);
void begin_frame(Gui*);
void end_frame(Gui*);
void render_gui(Gui*);

}  // namespace sai::debug