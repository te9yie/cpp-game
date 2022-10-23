#pragma once
#include "../task/fwd.h"
#include "../video/fwd.h"
#include "t9/defer.h"

namespace sai::debug {

// Gui.
struct Gui {
  t9::Defer shutdown_system;
};

bool init_gui_system(Gui* gui, const video::VideoSystem* video);
void begin_frame(Gui*);
void end_frame(Gui*);
void render_gui(video::VideoSystem* video, Gui*);
void preset_debug_gui(task::App* app);

}  // namespace sai::debug