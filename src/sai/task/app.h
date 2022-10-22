#pragma once
#include <memory>
#include <string_view>
#include <vector>

#include "context.h"
#include "event.h"
#include "runner.h"
#include "setup_task.h"
#include "t9/func_traits.h"
#include "t9/noncopyable.h"
#include "t9/type_list.h"
#include "task.h"

namespace sai::task {

// App.
class App : private t9::NonCopyable {
 private:
  Context context_;
  std::vector<std::unique_ptr<Event>> events_;
  std::vector<std::unique_ptr<SetupTask>> setup_tasks_;
  std::vector<std::shared_ptr<Task>> tasks_;

 public:
  App();

  template <typename F>
  void preset(F f) {
    f(this);
  }

  template <typename T, typename... Args>
  T* add_context(Args&&... args) {
    return context_.add<T>(std::forward<Args>(args)...);
  }

  template <typename T>
  void add_event() {
    auto e = std::make_unique<EventSubject<T>>();
    events_.emplace_back(std::move(e));
  }

  template <typename F>
  void add_setup_task(F f) {
    add_setup_task_(f, t9::args_type<F>{});
  }

  template <typename F>
  void add_task(std::string_view name, F f,
                const TaskOption& option = TaskOption{}) {
    add_task_(name, f, t9::args_type<F>{}, option);
  }

  bool run();

 private:
  template <typename F, typename... As>
  void add_setup_task_(F f, t9::type_list<As...>) {
    auto task = std::make_unique<FuncSetupTask<As...>>(f);
    setup_tasks_.emplace_back(std::move(task));
  }

  template <typename F, typename... As>
  void add_task_(std::string_view name, F f, t9::type_list<As...>,
                 const TaskOption& option) {
    auto task = std::make_shared<FuncTask<As...>>(name, f, option);
    tasks_.emplace_back(std::move(task));
  }

  void setup_dependencies_();
};

}  // namespace sai::task