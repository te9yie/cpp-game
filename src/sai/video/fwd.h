#pragma once
#include "../task/fwd.h"

namespace sai::video {

struct VideoSystem;
struct VideoSettings;
struct RenderSize;

void preset_video(task::App* app);

}  // namespace sai::video