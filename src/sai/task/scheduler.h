#pragma once
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
};

}  // namespace sai::task