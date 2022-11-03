#include "app.h"

#include <algorithm>

#include "../asset/fwd.h"
#include "../core/fwd.h"
#include "../debug/fwd.h"
#include "../ecs/registry.h"
#include "../graphics/fwd.h"
#include "../input/fwd.h"
#include "../video/fwd.h"
#include "../video/video.h"
#include "executor.h"
#include "scheduler.h"

namespace sai::task {

App::App() {
  if (auto scheduler = add_context<Scheduler>()) {
    scheduler->add_phase(make_phase<FirstPhase>("First"));
    scheduler->add_phase(make_phase<PreUpdatePhase>("PreUpdate"));
    scheduler->add_phase(make_phase<UpdatePhase>("Update"));
    scheduler->add_phase(make_phase<PostUpdatePhase>("PostUpdate"));
    scheduler->add_phase(make_phase<PreRenderPhase>("PreRender"));
    scheduler->add_phase(make_phase<RenderPhase>("Render"));
    scheduler->add_phase(make_phase<PostRenderPhase>("PostRender"));
    scheduler->add_phase(make_phase<LastPhase>("Last"));
  }

  preset(core::preset_core);
  preset(asset::preset_asset);
  preset(preset_executor);
  preset(video::preset_video);
  preset(input::preset_input);
  preset(graphics::preset_graphics);
  preset(debug::preset_debug_gui);
  add_context<ecs::Registry>();
}

bool App::run() {
  auto sch = context_.get<Scheduler>();
  sch->setup_task_dependencies();
  for (auto& task : setup_tasks_) {
    if (!task->exec(&context_)) {
      return false;
    }
  }
  std::size_t reader_index = 0;
  auto reader = EventReader<video::WindowEvent>(&context_, &reader_index);
  bool loop = true;
  while (loop) {
    reader.each([&loop](video::WindowEvent e) {
      if (e == video::WindowEvent::Quit) {
        loop = false;
      }
    });
    sch->run(&context_);
  }
  return true;
}

void App::add_task_(phase_index_type index, std::shared_ptr<Task> task) {
  auto sch = context_.get<Scheduler>();
  sch->add_task(index, std::move(task));
}

}  // namespace sai::task