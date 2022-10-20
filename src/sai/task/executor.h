#pragma once
#include <deque>
#include <memory>
#include <string_view>

#include "t9/func_traits.h"
#include "t9/noncopyable.h"
#include "task.h"

namespace sai::task {

// Executor.
class Executor : private t9::NonCopyable, private TaskObserver {
 private:
  std::deque<std::shared_ptr<Task>> tasks_;

 public:
  template <typename F>
  void add_task(std::string_view name, F f,
                const TaskOption& option = TaskOption{}) {
    add_task_(name, f, t9::args_type<F>{}, option);
  }

 private:
  // TaskObserver.
  virtual void on_pre_exec_task(Task* task) override;
  virtual void on_post_exec_task(Task* task) override;

 private:
  template <typename F, typename... As>
  void add_task_(std::string_view name, F f, t9::type_list<As...>,
                 const TaskOption& option) {
    auto task = std::make_shared<FuncTask<As...>>(name, f, option);
    add_task_(std::move(task));
  }
  void add_task_(std::shared_ptr<Task> task);
};

}  // namespace sai::task
