#pragma once
#include <bitset>
#include <cstddef>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <set>
#include <string_view>

#include "config.h"
#include "job.h"
#include "t9/dynamic_tuple.h"
#include "t9/function_traits.h"
#include "t9/noncopyable.h"
#include "t9/type_list.h"
#include "threading.h"

namespace sai {

// TaskContext.
using TaskContext = t9::DynamicTuple;

namespace task_ {

struct task_arg_type_index_ {
  static inline std::size_t index = 0;
};

}  // namespace task_

// task_arg_type_index.
template <typename T>
struct task_arg_type_index {
  static std::size_t index() {
    static std::size_t i = task_::task_arg_type_index_::index++;
    return i;
  }
};

// TaskArgsPermission.
struct TaskArgsPermission {
  std::set<std::size_t> writes;
  std::set<std::size_t> reads;

  bool is_conflict_write(std::size_t i) const {
    if (writes.find(i) != writes.end()) return true;
    if (reads.find(i) != reads.end()) return true;
    return false;
  }
  bool is_conflict_read(std::size_t i) const {
    if (writes.find(i) != writes.end()) return true;
    return false;
  }
};

// task_arg_traits.
template <typename T>
struct task_arg_traits;

template <typename T>
struct task_arg_traits<T*> {
  static void set_permission(TaskArgsPermission* p) {
    p->writes.emplace(task_arg_type_index<T>::index());
  }
  static T* from_context(const TaskContext& ctx) { return ctx.get<T>(); }
};

template <typename T>
struct task_arg_traits<const T*> {
  static void set_permission(TaskArgsPermission* p) {
    p->reads.emplace(task_arg_type_index<T>::index());
  }
  static const T* from_context(const TaskContext& ctx) { return ctx.get<T>(); }
};

// make_task_args_permission.
namespace task_ {

inline void set_task_args_permission_(TaskArgsPermission*, t9::type_list<>) {}

template <typename T, typename... Ts>
inline void set_task_args_permission_(TaskArgsPermission* p,
                                      t9::type_list<T, Ts...>) {
  task_arg_traits<T>::set_permission(p);
  set_task_args_permission_(p, t9::type_list<Ts...>{});
}

}  // namespace task_

template <typename... Ts>
inline TaskArgsPermission make_task_args_permission() {
  TaskArgsPermission p;
  task_::set_task_args_permission_(&p, t9::type_list<Ts...>{});
  return p;
}

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

  enum { FLAG_FENCE, FLAG_MAIN_THREAD, FLAG_MAX };

 private:
  TaskArgsPermission args_permission_;
  State state_ = State::None;
  std::bitset<FLAG_MAX> flags_;
  TaskObserver* observer_ = nullptr;
  std::deque<Task*> dependencies_;

 public:
  explicit Task(const TaskArgsPermission& permission);
  virtual ~Task() = default;

  void exec(const TaskContext& ctx);

  const TaskArgsPermission& args_permission() const { return args_permission_; }

  bool can_exec() const;
  bool is_wait_done() const { return state_ == State::WaitDone; }
  bool is_done() const { return state_ == State::Done; }
  bool is_fence() const { return flags_.test(FLAG_FENCE); }
  bool is_main_thread() const { return flags_.test(FLAG_MAIN_THREAD); }

  bool reset_state();
  bool set_submit();
  bool set_done();
  void set_fence() { flags_.set(FLAG_FENCE); }
  void set_main_thread() { flags_.set(FLAG_MAIN_THREAD); }

  void set_observer(TaskObserver* observer) { observer_ = observer; }
  void add_dependency(Task* task);

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
  FuncTask(F f) : Task(make_task_args_permission<As...>()), func_(f) {}

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
  void add_task(F f, const TaskOption& opt = TaskOption{}) {
    using args_type = typename t9::function_traits<F>::args_type;
    add_task_(f, args_type{}, opt);
  }

  void run();

 private:
  // TaskObserver.
  virtual void on_pre_exec_task(Task* task) override;
  virtual void on_post_exec_task(Task* task) override;

 private:
  template <typename F, typename... As>
  void add_task_(F f, t9::type_list<As...>, const TaskOption& opt) {
    std::shared_ptr<Task> task = std::make_shared<FuncTask<As...>>(f);
    if (opt.is_fence) {
      task->set_fence();
    }
    add_task(std::move(task));
  }
};

}  // namespace sai
