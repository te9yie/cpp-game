#pragma once
#include <SDL.h>

#include <bitset>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include "../job/job.h"
#include "args.h"
#include "context.h"
#include "t9/noncopyable.h"

namespace sai::task {

// TaskOption.
struct TaskOption {
  bool is_fence = false;
  SDL_threadID exclusive_thread_id = 0;

  TaskOption& fence() {
    is_fence = true;
    return *this;
  }
  TaskOption& exclusive_this_thread() {
    exclusive_thread_id = SDL_ThreadID();
    return *this;
  }
};

// Task.
class Task : private t9::NonCopyable, public job::Job {
 private:
  enum {
    FLAG_FENCE,
    FLAG_MAX,
  };

 private:
  ArgsTypeBits type_bits_;
  TaskWork work_;
  SDL_threadID exclusive_thread_id_ = 0;
  std::bitset<FLAG_MAX> flags_;
  std::deque<Task*> dependencies_;
  const AppContext* context_ = nullptr;

 public:
  Task(std::string_view name, const ArgsTypeBits& bits,
       const TaskOption& option);
  virtual ~Task() = default;

  void set_context(const AppContext* ctx) { context_ = ctx; }

  bool add_dependency(Task* task);
  const std::deque<Task*>& dependencies() const { return dependencies_; }

  const ArgsTypeBits& type_bits() const { return type_bits_; }
  bool is_fence() const { return flags_.test(FLAG_FENCE); }

 protected:
  virtual bool on_can_exec() const override;
  virtual void on_exec() override;
  virtual void on_exec_task(const AppContext* ctx, TaskWork* work) = 0;

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
  virtual void on_exec_task(const AppContext* ctx, TaskWork* work) override {
    func_(arg_traits<As>::to(ctx, work)...);
  }
};

// TaskList.
using TaskList = std::deque<std::shared_ptr<Task>>;

}  // namespace sai::task
