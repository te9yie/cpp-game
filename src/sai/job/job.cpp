#include "job.h"

namespace sai::job {

void Job::exec() {
  if (observer_) {
    observer_->on_pre_exec_job(this);
  }
  state_ = State::Exec;
  on_exec();
  state_ = State::WaitDone;
  if (observer_) {
    observer_->on_post_exec_job(this);
  }
}

bool Job::change_state(State state) {
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

}  // namespace sai::job
