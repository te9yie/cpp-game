#include "task.h"

#include <SDL.h>

namespace sai::task {

Task::Task(std::string_view name, const ArgsTypeBits& bits,
           const TaskOption& option)
    : name_(name), type_bits_(bits) {
  flags_.set(FLAG_FENCE, option.is_fence);
}

bool Task::can_exec() const {
  for (auto task : dependencies_) {
    if (!task->is_state(State::Done)) return false;
  }
  return true;
}
void Task::exec(const Context* ctx) {
  if (observer_) {
    observer_->on_pre_exec_task(this);
  }
  state_ = State::Exec;
  on_exec(ctx, &work_);
  state_ = State::WaitDone;
  if (observer_) {
    observer_->on_post_exec_task(this);
  }
}

bool Task::change_state(State state) {
  if (state_ == state) return true;

  bool is_ok = false;
  switch (state) {
    case State::None:
      is_ok = state_ == State::Done;
      break;
    case State::Submit:
      is_ok = state_ == State::None;
      break;
    case State::Exec:
      is_ok = state_ == State::Submit;
      break;
    case State::WaitDone:
      is_ok = state_ == State::Exec;
      break;
    case State::Done:
      is_ok = state_ == State::WaitDone;
      break;
  }
  if (is_ok) {
    state_ = state;
  }
  return is_ok;
}

bool Task::add_dependency(Task* task) {
  if (is_depended_(task)) return false;
  dependencies_.emplace_back(task);
  return true;
}

bool Task::set_observer(TaskObserver* observer) {
  if (observer) {
    SDL_assert(!observer_);
    if (observer_) return false;
    observer_ = observer;
  } else {
    observer_ = nullptr;
  }
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
