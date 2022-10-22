#include "app.h"

#include <algorithm>
#include <iterator>

#include "executor.h"
#include "sai/job/executor.h"

namespace sai::task {

App::App() { preset(preset_executor); }

bool App::run() {
  setup_dependencies_();
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
    std::for_each(tasks_.begin(), tasks_.end(), [&](auto& task) {
      task->set_context(&context_);
      task->reset_state();
      executor->submit(task);
    });
    executor->kick();
    executor->join();
  }
  return true;
}

void App::setup_dependencies_() {
  for (auto it = tasks_.begin(), it_end = tasks_.end(); it != it_end; ++it) {
    auto task = it->get();
    if (task->is_fence()) {
      for (auto r_it = std::reverse_iterator<decltype(it)>(it),
                r_it_end = tasks_.rend();
           r_it != r_it_end; ++r_it) {
        task->add_dependency(r_it->get());
        if ((*r_it)->is_fence()) break;
      }
    } else {
      const auto& bits = task->type_bits();
      for (auto i : bits.writes) {
        for (auto r_it = std::reverse_iterator<decltype(it)>(it),
                  r_it_end = tasks_.rend();
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
                  r_it_end = tasks_.rend();
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