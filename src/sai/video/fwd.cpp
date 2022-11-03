#include "fwd.h"

#include "../task/app.h"
#include "../task/task.h"
#include "video.h"

namespace sai::video {

void preset_video(task::App* app) {
  app->add_context<VideoSystem>();
  app->add_context<RenderSize>();

  app->add_event<WindowEvent>();

  app->add_setup_task(init_video_system);
  app->add_task_in_phase<task::FirstPhase>(
      "handle events", handle_events,
      task::TaskOption().exclusive_this_thread());
  app->add_task_in_phase<task::PreRenderPhase>("== begin render", begin_render);
  app->add_task_in_phase<task::LastPhase>(
      "== end render", end_render, task::TaskOption().exclusive_this_thread());
}

}  // namespace sai::video