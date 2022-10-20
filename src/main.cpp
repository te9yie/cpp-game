#include <SDL.h>

#include "imgui.h"
#include "sai/core/core.h"
#include "sai/debug/gui.h"
#include "sai/task/executor.h"
#include "sai/video/video.h"

int main(int /*argc*/, char* /*argv*/[]) {
  sai::task::Context context;
  auto tasks = context.add<sai::task::Executor>("TaskExecutor");
  tasks->setup(2);

  context.add<sai::core::System>();
  context.add<sai::video::VideoSystem>();
  context.add<sai::debug::Gui>();
  context.add<sai::task::ExecutorWork>();

  tasks->add_setup_task(sai::core::init_system);
  tasks->add_setup_task(sai::video::init_video_system);
  tasks->add_setup_task(sai::debug::init_gui_system);

  using Option = sai::task::TaskOption;

  tasks->add_task("handle events", sai::core::handle_events,
                  Option().exclusive_this_thread().fence());
  tasks->add_task("begin debug gui", sai::debug::begin_frame, Option().fence());
  tasks->add_task("render debug task",
                  [](sai::debug::Gui*, sai::task::Executor* tasks) {
                    ImGui::Begin("Debug");
                    tasks->render_debug_gui();
                    ImGui::End();
                  });
  tasks->add_task("render demo",
                  [](sai::debug::Gui*) { ImGui::ShowDemoWindow(); });
  tasks->add_task("end debug gui", sai::debug::end_frame, Option().fence());
  tasks->add_task("begin render", sai::video::begin_render, Option().fence());
  tasks->add_task("render debug gui", sai::debug::render_gui);
  tasks->add_task("end render", sai::video::end_render, Option().fence());

  return tasks->run(&context) ? EXIT_SUCCESS : EXIT_FAILURE;
}
