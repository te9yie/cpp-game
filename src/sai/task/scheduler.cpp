#include "scheduler.h"

#include <algorithm>
#include <cassert>

namespace sai::task {

void Scheduler::setup_task_dependencies() {
  std::for_each(phases.begin(), phases.end(),
                [](auto& phase) { phase->setup_task_dependencies(); });
}

void Scheduler::add_task(phase_index_type index, std::shared_ptr<Task> task) {
  auto it = std::find_if(phases.begin(), phases.end(), [index](auto& phase) {
    return phase->index == index;
  });
  assert(it != phases.end() && "not found phase.");
  (*it)->tasks.emplace_back(std::move(task));
}

void Scheduler::run(const Context* ctx) {
  std::for_each(phases.begin(), phases.end(),
                [ctx](auto& phase) { phase->run(ctx); });
}

void Scheduler::render_debug_gui() {
  std::for_each(phases.begin(), phases.end(),
                [](auto& phase) { phase->render_debug_gui(); });
}

}  // namespace sai::task