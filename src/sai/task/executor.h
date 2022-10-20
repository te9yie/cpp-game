#pragma once
#include <deque>
#include <memory>
#include <string_view>

#include "sai/job.h"
#include "sai/threading.h"
#include "t9/func_traits.h"
#include "t9/noncopyable.h"
#include "task.h"

#if defined(_DEBUG)
#include <map>
#endif

namespace sai::task {

class Context;

// Executor.
class Executor : private t9::NonCopyable, private TaskObserver {
 private:
  job::Executor executor_;
  std::deque<std::unique_ptr<Task>> tasks_;
  std::deque<Task*> wait_tasks_;
  std::deque<Task*> active_tasks_;
  MutexPtr mutex_;
  ConditionPtr condition_;
  volatile bool notify_ = false;

 public:
  explicit Executor(std::string_view name);
  virtual ~Executor() override;

  bool setup(std::size_t thread_n);
  void tear_down();

  template <typename F>
  void add_task(std::string_view name, F f,
                const TaskOption& option = TaskOption{}) {
    add_task_(name, f, t9::args_type<F>{}, option);
  }

  void run(const Context* ctx);

 private:
  // TaskObserver.
  virtual void on_pre_exec_task(Task* task) override;
  virtual void on_post_exec_task(Task* task) override;

 private:
  template <typename F, typename... As>
  void add_task_(std::string_view name, F f, t9::type_list<As...>,
                 const TaskOption& option) {
    auto task = std::make_unique<FuncTask<As...>>(name, f, option);
    add_task_(std::move(task));
  }
  void add_task_(std::unique_ptr<Task> task);

#if defined(_DEBUG)
 private:
  bool refreshed_ = false;
  std::multimap<int, Task*> task_order_;
  int max_order_ = 0;

 public:
  void render_debug_gui();
#endif
};

}  // namespace sai::task
