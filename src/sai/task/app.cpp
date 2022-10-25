#include "app.h"

#include <algorithm>
#include <iterator>

#include "../core/core.h"
#include "../debug/gui.h"
#include "../graphics/sprite.h"
#include "../video/video.h"
#include "executor.h"

namespace sai::task {

App::App() {
  phases_.emplace_back(make_phase<FirstPhase>("First"));
  phases_.emplace_back(make_phase<PreUpdatePhase>("PreUpdate"));
  phases_.emplace_back(make_phase<UpdatePhase>("Update"));
  phases_.emplace_back(make_phase<PostUpdatePhase>("PostUpdate"));
  phases_.emplace_back(make_phase<PreRenderPhase>("PreRender"));
  phases_.emplace_back(make_phase<RenderPhase>("Render"));
  phases_.emplace_back(make_phase<PostRenderPhase>("PostRender"));
  phases_.emplace_back(make_phase<LastPhase>("Last"));

  add_context<PhaseReference>(PhaseReference{&phases_});

  preset(core::preset_core);
  preset(preset_executor);
  preset(video::preset_video);
  preset(graphics::preset_graphics);
  preset(debug::preset_debug_gui);
}

bool App::run() {
  std::for_each(phases_.begin(), phases_.end(),
                [this](auto& phase) { phase->setup_task_dependencies(); });
  for (auto& task : setup_tasks_) {
    if (!task->exec(&context_)) {
      return false;
    }
  }
  std::size_t reader_index = 0;
  auto reader = EventReader<video::WindowEvent>(&context_, &reader_index);
  bool loop = true;
  while (loop) {
    std::for_each(events_.begin(), events_.end(), [](auto e) { e->update(); });
    reader.each([&loop](video::WindowEvent e) {
      if (e == video::WindowEvent::Quit) {
        loop = false;
      }
    });
    std::for_each(phases_.begin(), phases_.end(),
                  [&](auto& phase) { phase->run(&context_); });
  }
  return true;
}

void App::add_task_(phase_index_type index, std::shared_ptr<Task> task) {
  auto it = std::find_if(phases_.begin(), phases_.end(), [index](auto& phase) {
    return phase->index == index;
  });
  assert(it != phases_.end() && "not found phase.");
  (*it)->tasks.emplace_back(std::move(task));
}

}  // namespace sai::task