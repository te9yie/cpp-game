#pragma once
#include <bitset>
#include <deque>
#include <functional>
#include <string>
#include <string_view>

#include "args.h"
#include "context.h"
#include "t9/noncopyable.h"

namespace sai::task {

class Task;

// TaskObserver.
class TaskObserver {
 public:
  virtual ~TaskObserver() = default;
  virtual void on_pre_exec_task(Task* task) = 0;
  virtual void on_post_exec_task(Task* task) = 0;
};

// TaskOption.
struct TaskOption {
  bool is_fence = false;

  TaskOption& set_fence() {
    is_fence = true;
    return *this;
  }
};

// Task.
class Task : private t9::NonCopyable {
 private:
  enum {
    FLAG_FENCE,
    FLAG_MAX,
  };

 public:
  enum class State {
    None,
    Submit,
    Exec,
    WaitDone,
    Done,
  };

 private:
  std::string name_;
  ArgsTypeBits type_bits_;
  TaskWork work_;
  State state_ = State::None;
  std::bitset<FLAG_MAX> flags_;
  std::deque<Task*> dependencies_;
  TaskObserver* observer_ = nullptr;

 public:
  Task(std::string_view name, const ArgsTypeBits& bits,
       const TaskOption& option);
  virtual ~Task() = default;

  bool can_exec() const;
  void exec(const Context* ctx);

  bool change_state(State state);
  bool reset_state() { return change_state(State::None); }

  bool add_dependency(Task* task);
  bool set_observer(TaskObserver* observer);

  const std::string& name() const { return name_; }
  const ArgsTypeBits& type_bits() const { return type_bits_; }
  bool is_state(State state) const { return state_ == state; }
  bool is_fence() const { return flags_.test(FLAG_FENCE); }

 protected:
  virtual void on_exec(const Context* ctx, TaskWork* work) = 0;

 private:
  bool is_depended_(Task* task) const;
};

// FuncTask.
template <typename... As>
class FuncTask : public Task {
 public:
  using FuncType = std::function<void(As...)>;

 private:
  FuncType func_;

 public:
  FuncTask(std::string_view name, const FuncType& f, const TaskOption& option)
      : Task(name, make_args_type_bits<As...>(), option), func_(f) {}

 protected:
  virtual void on_exec(const Context* ctx, TaskWork* work) override {
    func_(arg_traits<As>::to(ctx, work)...);
  }
};

}  // namespace sai::task
