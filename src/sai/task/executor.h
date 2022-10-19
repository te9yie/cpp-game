#pragma once
#include <deque>
#include <memory>

#include "t9/func_traits.h"
#include "t9/noncopyable.h"
#include "task.h"

namespace sai::task {

// Executor.
class Executor : private t9::NonCopyable {
 private:
  std::deque<std::shared_ptr<Task>> tasks_;

 public:
  template <typename F>
  void add_task(F f) {
    add_task_(f, t9::args_type<F>{});
  }

 private:
  template <typename F, typename... As>
  void add_task_(F f, t9::type_list<As...>) {
    auto task = std::make_shared<FuncTask<As...>>(f);
    add_task_(std::move(task));
  }
  void add_task_(std::shared_ptr<Task> task);
};

}  // namespace sai::task