#include <SDL.h>

#include "imgui.h"
#include "sai/core/core.h"
#include "sai/core/frame.h"
#include "sai/debug/gui.h"
#include "sai/debug/performance.h"
#include "sai/task/executor.h"
#include "sai/video/video.h"

int main(int /*argc*/, char* /*argv*/[]) {
  sai::task::Context context;
  context.add<sai::debug::PerformanceProfiler>();
  auto tasks = context.add<sai::task::Executor>("TaskExecutor");
  tasks->setup(2);

  context.add<sai::core::System>();
  context.add<sai::video::VideoSystem>();
  context.add<sai::debug::Gui>();
  context.add<sai::task::ExecutorWork>();
  context.add<sai::core::Frame>();

  tasks->add_setup_task("init system", sai::core::init_system);
  tasks->add_setup_task("init vidoe system", sai::video::init_video_system);
  tasks->add_setup_task("init gui system", sai::debug::init_gui_system);
  tasks->add_setup_task("init profiler",
                        [](sai::debug::PerformanceProfiler* profiler) {
                          profiler->setup_thread("MainThread");
                          return true;
                        });

  using Option = sai::task::TaskOption;

  tasks->add_task(
      "performance tick",
      [](sai::debug::PerformanceProfiler* profiler) { profiler->tick(); });
  tasks->add_task("frame tick", sai::core::tick_frame);
  tasks->add_task("handle events", sai::core::handle_events,
                  Option().exclusive_this_thread().fence());
  tasks->add_task("-- begin debug gui", sai::debug::begin_frame,
                  Option().fence());
  tasks->add_task("render debug gui",
                  [](sai::debug::Gui*, sai::task::Executor* tasks,
                     sai::debug::PerformanceProfiler* profiler,
                     const sai::core::Frame* frame) {
                    ImGui::Begin("Debug");

                    auto delta = frame->start_count - frame->last_start_count;
                    auto delta_ms = delta * 1000 / frame->frequency;
                    ImGui::Text("delta (count): %lu", delta);
                    ImGui::Text("delta (ms): %lu", delta_ms);
                    ImGui::Text("frame count: %lu", frame->frame_count);

                    ImGui::Separator();

                    tasks->render_debug_gui();
                    profiler->render_debug_gui();
                    ImGui::End();
                  });
  tasks->add_task("-- end debug gui", sai::debug::end_frame, Option().fence());
  tasks->add_task("== begin render", sai::video::begin_render,
                  Option().fence());
  tasks->add_task("render debug gui", sai::debug::render_gui);
  tasks->add_task("== end render", sai::video::end_render,
                  Option().exclusive_this_thread().fence());

  return tasks->run(&context) ? EXIT_SUCCESS : EXIT_FAILURE;
}
