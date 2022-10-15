#pragma once
#include <bitset>
#include <cstddef>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <string_view>

#include "config.h"
#include "job.h"
#include "t9/function_traits.h"
#include "t9/noncopyable.h"
#include "t9/type_list.h"
#include "task_arg.h"
#include "threading.h"

namespace sai {

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
  enum class State {
    None,
    Submit,
    Exec,
    WaitDone,
    Done,
  };

  enum { FLAG_FENCE, FLAG_MAX };

 private:
  std::string name_;
  TaskArgsPermission args_permission_;
  State state_ = State::None;
  std::bitset<FLAG_MAX> flags_;
  TaskObserver* observer_ = nullptr;
  std::deque<Task*> dependencies_;

 public:
  Task(std::string_view name, const TaskArgsPermission& permission);
  virtual ~Task() = default;

  void set_fence() { flags_.set(FLAG_FENCE); }

  void exec(const TaskContext& ctx);

  const TaskArgsPermission& args_permission() const { return args_permission_; }

  const std::string& name() const { return name_; }
  bool can_exec() const;
  bool is_wait_done() const { return state_ == State::WaitDone; }
  bool is_done() const { return state_ == State::Done; }
  bool is_fence() const { return flags_.test(FLAG_FENCE); }

  bool reset_state();
  bool set_submit();
  bool set_done();

  void set_observer(TaskObserver* observer) { observer_ = observer; }

  void add_dependency(Task* task);
  const std::deque<Task*>& dependencies() const { return dependencies_; }

 protected:
  virtual void on_exec(const TaskContext& ctx) = 0;

 private:
  bool is_depended_(Task* task) const;
};

// FuncTask.
template <typename... As>
class FuncTask : public Task {
 private:
  std::function<void(As...)> func_;

 public:
  template <typename F>
  FuncTask(std::string_view name, F f)
      : Task(name, make_task_args_permission<As...>()), func_(f) {}

 protected:
  virtual void on_exec(const TaskContext& ctx) override {
    func_(task_arg_traits<As>::from_context(ctx)...);
  }
};

// TaskExecutor.
class TaskExecutor : private TaskObserver, private t9::NonMovable {
 private:
  JobExecutor executor_;
  TaskContext context_;
  std::deque<std::shared_ptr<Task>> tasks_;
  std::list<Task*> wait_tasks_;
  std::list<Task*> active_tasks_;
  MutexPtr mutex_;
  ConditionPtr condition_;
  volatile bool notify_ = false;

 public:
  explicit TaskExecutor(std::string_view name);
  virtual ~TaskExecutor() override;

  bool setup(std::size_t thread_n);
  void tear_down();

  template <typename T, typename... Args>
  void add_context(Args&&... args) {
    context_.set<T>(std::forward<Args>(args)...);
  }

  void add_task(std::shared_ptr<Task> task);
  template <typename F>
  void add_task(std::string_view name, F f,
                const TaskOption& opt = TaskOption{}) {
    using args_type = typename t9::function_traits<F>::args_type;
    add_task_(name, f, args_type{}, opt);
  }

  void run();

 private:
  // TaskObserver.
  virtual void on_pre_exec_task(Task* task) override;
  virtual void on_post_exec_task(Task* task) override;

 private:
  template <typename F, typename... As>
  void add_task_(std::string_view name, F f, t9::type_list<As...>,
                 const TaskOption& opt) {
    std::shared_ptr<Task> task = std::make_shared<FuncTask<As...>>(name, f);
    if (opt.is_fence) {
      task->set_fence();
    }
    add_task(std::move(task));
  }

 public:
  void render_debug_gui();
};

}  // namespace sai
