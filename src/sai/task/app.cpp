#include "app.h"

#include <algorithm>
#include <iterator>

#include "../core/core.h"
#include "../debug/gui.h"
#include "../video/video.h"
#include "executor.h"

namespace sai::task {

App::App() {
  phase_.emplace_back(make_phase<FirstPhase>());
  phase_.emplace_back(make_phase<PreUpdatePhase>());
  phase_.emplace_back(make_phase<UpdatePhase>());
  phase_.emplace_back(make_phase<PostUpdatePhase>());
  phase_.emplace_back(make_phase<PreRenderPhase>());
  phase_.emplace_back(make_phase<RenderPhase>());
  phase_.emplace_back(make_phase<PostRenderPhase>());
  phase_.emplace_back(make_phase<LastPhase>());

  preset(core::preset_core);
  preset(preset_executor);
  preset(video::preset_video);
  preset(debug::preset_debug_gui);
}

bool App::run() {
  std::for_each(phase_.begin(), phase_.end(),
                [this](auto& phase) { setup_dependencies_(phase.get()); });
  for (auto& task : setup_tasks_) {
    if (!task->exec(&context_)) {
      return false;
    }
  }
  auto executor = context_.get<job::Executor>();
  while (executor) {
    if (auto work = context_.get<ExecutorWork>()) {
      if (!work->loop) break;
    }
    std::for_each(phase_.begin(), phase_.end(), [&](auto& phase) {
      std::for_each(phase->tasks.begin(), phase->tasks.end(), [&](auto& task) {
        task->set_context(&context_);
        task->reset_state();
        executor->submit(task);
      });
      executor->kick();
      executor->join();
    });
  }
  return true;
}

void App::add_task_(phase_index_type index, std::shared_ptr<Task> task) {
  auto it = std::find_if(phase_.begin(), phase_.end(), [index](auto& phase) {
    return phase->index == index;
  });
  assert(it != phase_.end() && "not found phase.");
  (*it)->tasks.emplace_back(std::move(task));
}

void App::setup_dependencies_(Phase* phase) {
  for (auto it = phase->tasks.begin(), it_end = phase->tasks.end();
       it != it_end; ++it) {
    auto task = it->get();
    if (task->is_fence()) {
      for (auto r_it = std::reverse_iterator<decltype(it)>(it),
                r_it_end = phase->tasks.rend();
           r_it != r_it_end; ++r_it) {
        task->add_dependency(r_it->get());
        if ((*r_it)->is_fence()) break;
      }
    } else {
      const auto& bits = task->type_bits();
      for (auto i : bits.writes) {
        for (auto r_it = std::reverse_iterator<decltype(it)>(it),
                  r_it_end = phase->tasks.rend();
             r_it != r_it_end; ++r_it) {
          if ((*r_it)->is_fence()) {
            task->add_dependency(r_it->get());
            break;
          }
          if ((*r_it)->type_bits().is_conflict_write(i)) {
            task->add_dependency(r_it->get());
          }
        }
      }
      for (auto i : bits.reads) {
        for (auto r_it = std::reverse_iterator<decltype(it)>(it),
                  r_it_end = phase->tasks.rend();
             r_it != r_it_end; ++r_it) {
          if ((*r_it)->is_fence()) {
            task->add_dependency(r_it->get());
            break;
          }
          if ((*r_it)->type_bits().is_conflict_read(i)) {
            task->add_dependency(r_it->get());
          }
        }
      }
    }
  }
}

}  // namespace sai::task