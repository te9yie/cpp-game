#include "executor.h"

namespace sai::task {

/*virtual*/ void Executor::on_pre_exec_task(Task*) /*override*/ {}
/*virtual*/ void Executor::on_post_exec_task(Task*) /*override*/ {}

void Executor::add_task_(std::shared_ptr<Task> task) {
  // setup dependencies.
  if (task->is_fence()) {
    for (auto it = tasks_.rbegin(), last = tasks_.rend(); it != last; ++it) {
      task->add_dependency(it->get());
      if ((*it)->is_fence()) break;
    }
  } else {
    const auto& bits = task->type_bits();
    for (auto i : bits.writes) {
      for (auto it = tasks_.rbegin(), last = tasks_.rend(); it != last; ++it) {
        if ((*it)->is_fence()) {
          task->add_dependency(it->get());
          break;
        }
        if ((*it)->type_bits().is_conflict_write(i)) {
          task->add_dependency(it->get());
        }
      }
    }
    for (auto i : bits.reads) {
      for (auto it = tasks_.rbegin(), last = tasks_.rend(); it != last; ++it) {
        if ((*it)->is_fence()) {
          task->add_dependency(it->get());
          break;
        }
        if ((*it)->type_bits().is_conflict_read(i)) {
          task->add_dependency(it->get());
        }
      }
    }
  }
  // add task.
  task->set_observer(this);
  tasks_.emplace_back(std::move(task));
}

}  // namespace sai::task
