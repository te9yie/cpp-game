#include "task.h"

#include <SDL.h>

namespace sai::task {

Task::Task(std::string_view name, const ArgsTypeBits& bits,
           const TaskOption& option)
    : job::Job(name), type_bits_(bits) {
  flags_.set(FLAG_FENCE, option.is_fence);
  exclusive_thread_id_ = option.exclusive_thread_id;
}

/*virtual*/ bool Task::on_can_exec() const /*override*/ {
  for (auto task : dependencies_) {
    if (!task->is_state(State::Done)) return false;
  }
  if (exclusive_thread_id_ != 0) {
    if (exclusive_thread_id_ != SDL_ThreadID()) return false;
  }
  return true;
}
/*virtual*/ void Task::on_exec() /*override*/ {
  on_exec_task(context_, &work_);
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
