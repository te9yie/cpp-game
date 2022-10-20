#include "executor.h"

#include <algorithm>
#include <iterator>

#if defined(_DEBUG)
#include <map>

#include "imgui.h"
#endif

namespace sai::task {

/*explicit*/ Executor::Executor(std::string_view name) : executor_(name) {}
/*virtual*/ Executor::~Executor() { tear_down(); }

bool Executor::setup(std::size_t thread_n) {
  if (!tasks_.empty()) return false;

  MutexPtr mutex(SDL_CreateMutex());
  if (!mutex) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[task::Executor::setup] %s",
                 SDL_GetError());
    return false;
  }

  ConditionPtr condition(SDL_CreateCond());
  if (!condition) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[task::Executor::setup] %s",
                 SDL_GetError());
    return false;
  }

  if (!executor_.start(thread_n)) return false;

  mutex_ = std::move(mutex);
  condition_ = std::move(condition);

  return true;
}
void Executor::tear_down() {
  executor_.stop();
  tasks_.clear();
  mutex_.reset();
  condition_.reset();
}

bool Executor::run(const Context* ctx) {
  for (auto& task : setup_tasks_) {
    if (!task->exec(ctx)) {
      return false;
    }
  }
  while (true) {
    if (auto work = ctx->get<ExecutorWork>()) {
      if (!work->loop) break;
    }
    std::for_each(tasks_.begin(), tasks_.end(), [&](auto& task) {
      task->set_context(ctx);
      task->reset_state();
      executor_.submit(task);
    });
    executor_.kick();
    executor_.join();
  }
  return true;
}

void Executor::add_setup_task_(std::shared_ptr<SetupTask> task) {
  // add task.
  setup_tasks_.emplace_back(std::move(task));
}

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
  tasks_.emplace_back(std::move(task));

#if defined(_DEBUG)
  refreshed_ = false;
#endif
}

#if defined(_DEBUG)
void Executor::render_debug_gui() {
  if (!refreshed_) {
    refreshed_ = true;
    std::map<Task*, int> task_depth;
    int max_depth = 0;
    for (auto& task : tasks_) {
      int depth = 0;
      if (!task->dependencies().empty()) {
        for (auto& d_task : task->dependencies()) {
          if (auto it = task_depth.find(d_task); it != task_depth.end()) {
            depth = std::max(it->second + 1, depth);
          }
        }
      }
      max_depth = std::max(depth, max_depth);
      task_depth.emplace(task.get(), depth);
    }

    task_order_.clear();
    for (auto it : task_depth) {
      task_order_.emplace(it.second, it.first);
    }
    max_order_ = max_depth;
  }

  for (int i = 0; i <= max_order_; ++i) {
    ImGui::Text("%d:", i);
    auto range = task_order_.equal_range(i);
    for (auto it = range.first; it != range.second; ++it) {
      ImGui::SameLine();
      ImGui::Text("[%s]", it->second->name().c_str());
    }
  }
}
#endif

}  // namespace sai::task
