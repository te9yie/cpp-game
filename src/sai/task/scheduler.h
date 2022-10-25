#pragma once
#include <list>
#include <memory>

#include "phase.h"

namespace sai::task {

class Context;

// Scheduler.
struct Scheduler {
  std::list<std::unique_ptr<Phase>> phases;

  void setup_task_dependencies();
  void add_task(phase_index_type index, std::shared_ptr<Task> task);
  void run(const Context* ctx);
  void render_debug_gui();
};

}  // namespace sai::task