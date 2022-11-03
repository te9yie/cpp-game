#pragma once
#include <iterator>
#include <list>
#include <memory>

#include "context.h"
#include "phase.h"

namespace sai::task {

// Scheduler.
struct Scheduler {
  std::list<std::unique_ptr<Phase>> phases;

  void setup_task_dependencies();
  void add_task(phase_index_type index, std::shared_ptr<Task> task);
  void run(const AppContext* ctx);
  void render_debug_gui();

  void add_phase(std::unique_ptr<Phase> phase) {
    phases.push_back(std::move(phase));
  }

  template <typename T>
  void add_phase_before(std::unique_ptr<Phase> phase) {
    auto i = phase_index<T>::index();
    auto it = std::find_if(phases.begin(), phases.end(),
                           [i](auto& phase) { return phase->index == i; });
    if (it == phases.end()) {
      phases.push_front(std::move(phase));
    } else {
      phases.insert(it, std::move(phase));
    }
  }
};

}  // namespace sai::task