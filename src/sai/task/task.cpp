#include "task.h"

namespace sai::task {

/*explicit*/ Task::Task(const ArgsTypeBits& bits) : type_bits_(bits) {}

void Task::exec(const Context* ctx) { on_exec(ctx, &work_); }

bool Task::can_exec() const {
  for (auto task : dependencies_) {
    if (!task->is_done()) return false;
  }
  return true;
}

bool Task::add_dependency(Task* task) {
  if (is_depended_(task)) return false;
  dependencies_.emplace_back(task);
  return true;
}

bool Task::is_depended_(Task* task) const {
  for (auto it : dependencies_) {
    if (it == task) return true;
    if (it->is_depended_(task)) return true;
  }
  return false;
}

}  // namespace sai::task