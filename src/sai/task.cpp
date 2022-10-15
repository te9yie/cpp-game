#include "task.h"

#include <algorithm>
#include <iterator>

namespace sai {

// Task.
Task::Task(std::string_view name, const TaskArgsPermission& permission)
    : name_(name), args_permission_(permission) {}

void Task::exec(const TaskContext& ctx) {
  if (observer_) {
    observer_->on_pre_exec_task(this);
  }
  state_ = State::Exec;
  on_exec(ctx);
  state_ = State::WaitDone;
  if (observer_) {
    observer_->on_post_exec_task(this);
  }
}

bool Task::can_exec() const {
  for (auto task : dependencies_) {
    if (!task->is_done()) return false;
  }
  return true;
}

bool Task::reset_state() {
  if (state_ == State::None) return true;
  if (state_ == State::Done) {
    state_ = State::None;
    return true;
  }
  return false;
}
bool Task::set_submit() {
  if (state_ == State::None) {
    state_ = State::Submit;
    return true;
  }
  return false;
}
bool Task::set_done() {
  if (state_ == State::WaitDone) {
    state_ = State::Done;
    return true;
  }
  return false;
}

void Task::add_dependency(Task* task) {
  if (is_depended_(task)) return;
  dependencies_.emplace_back(task);
}

bool Task::is_depended_(Task* task) const {
  for (auto it : dependencies_) {
    if (task == it) return true;
    if (it->is_depended_(task)) return true;
  }
  return false;
}

// TaskExecutor.
/*explicit*/ TaskExecutor::TaskExecutor(std::string_view name)
    : executor_(name) {}
/*virtual*/ TaskExecutor::~TaskExecutor() /*override*/ { tear_down(); }

bool TaskExecutor::setup(std::size_t thread_n) {
  if (!tasks_.empty()) return false;

  MutexPtr mutex(SDL_CreateMutex());
  if (!mutex) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[TaskExecutor::setup] %s",
                 SDL_GetError());
    return false;
  }

  ConditionPtr condition(SDL_CreateCond());
  if (!condition) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[TaskExecutor::setup] %s",
                 SDL_GetError());
    return false;
  }

  if (!executor_.start(thread_n)) return false;

  mutex_ = std::move(mutex);
  condition_ = std::move(condition);

  return true;
}
void TaskExecutor::tear_down() {
  executor_.stop();
  wait_tasks_.clear();
  active_tasks_.clear();
  tasks_.clear();
  mutex_.reset();
  condition_.reset();
}

void TaskExecutor::add_task(std::shared_ptr<Task> task) {
  // setup dependencies.
  if (task->is_fence()) {
    for (auto it = tasks_.rbegin(), last = tasks_.rend(); it != last; ++it) {
      const auto& t = *it;
      task->add_dependency(t.get());
      if (t->is_fence()) break;
    }
  } else {
    const auto& permission = task->args_permission();
    for (auto i : permission.writes) {
      for (auto it = tasks_.rbegin(), last = tasks_.rend(); it != last; ++it) {
        const auto& t = *it;
        if (t->is_fence()) {
          task->add_dependency(t.get());
          break;
        }
        if (t->args_permission().is_conflict_write(i)) {
          task->add_dependency(t.get());
        }
      }
    }
    for (auto i : permission.reads) {
      for (auto it = tasks_.rbegin(), last = tasks_.rend(); it != last; ++it) {
        const auto& t = *it;
        if (t->is_fence()) {
          task->add_dependency(t.get());
          break;
        }
        if (t->args_permission().is_conflict_read(i)) {
          task->add_dependency(t.get());
        }
      }
    }
  }

  // add task.
  task->set_observer(this);
  tasks_.emplace_back(std::move(task));
}

void TaskExecutor::run() {
  std::for_each(tasks_.begin(), tasks_.end(),
                [](auto& task) { task->reset_state(); });
  std::transform(tasks_.begin(), tasks_.end(), std::back_inserter(wait_tasks_),
                 [](auto& task) { return task.get(); });

  while (!(wait_tasks_.empty() && active_tasks_.empty())) {
    {
      for (auto it = wait_tasks_.begin(); it != wait_tasks_.end();) {
        auto task = *it;
        if (!task->can_exec()) {
          ++it;
          continue;
        }
        task->set_submit();
        active_tasks_.emplace_back(task);
        executor_.submit_func([this, task]() { task->exec(context_); });
        it = wait_tasks_.erase(it);
      }
    }
    {
      UniqueLock lock(mutex_.get());
      while (!notify_) {
        SDL_CondWait(condition_.get(), mutex_.get());
      }
      notify_ = false;
    }
    for (auto it = active_tasks_.begin(); it != active_tasks_.end();) {
      auto task = *it;
      if (!task->is_wait_done()) {
        ++it;
        continue;
      }
      task->set_done();
      it = active_tasks_.erase(it);
    }
  }
}

/*virtual*/ void TaskExecutor::on_pre_exec_task(Task* /*task*/) /*override*/
{}
/*virtual*/ void TaskExecutor::on_post_exec_task(Task* /*task*/) /*override*/
{
  UniqueLock lock(mutex_.get());
  notify_ = true;
  SDL_CondSignal(condition_.get());
}

void TaskExecutor::render_debug_gui() {
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

  std::multimap<int, Task*> render_tasks;
  for (auto it : task_depth) {
    render_tasks.emplace(it.second, it.first);
  }

  for (int i = 0; i <= max_depth; ++i) {
    ImGui::Text("%d:", i);
    auto range = render_tasks.equal_range(i);
    for (auto it = range.first; it != range.second; ++it) {
      ImGui::SameLine();
      ImGui::Text("[%s]", it->second->name().c_str());
    }
  }

  /*
  auto canvas_p0 = ImGui::GetCursorScreenPos();
  auto canvas_s = ImGui::GetContentRegionAvail();
  if (canvas_s.x < 50.0f) canvas_s.x = 50.0f;
  if (canvas_s.y < 50.0f) canvas_s.y = 50.0f;
  auto canvas_p1 = ImVec2(canvas_p0.x + canvas_s.x, canvas_p0.y + canvas_s.y);

  auto draw = ImGui::GetWindowDrawList();
  draw->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(0x30, 0x30, 0x30, 0xff));
  draw->AddRect(canvas_p0, canvas_p1, IM_COL32(0xff, 0xff, 0xff, 0xff));
  */
}

}  // namespace sai
