#pragma once
#include <bitset>
#include <deque>
#include <functional>

#include "args.h"
#include "context.h"
#include "t9/noncopyable.h"

namespace sai::task {

// Task.
class Task : private t9::NonCopyable {
 private:
  enum class State {
    None,
    Done,
  };

  enum {
    FLAG_FENCE,
    FLAG_MAX,
  };

 private:
  ArgsTypeBits type_bits_;
  TaskWork work_;
  State state_ = State::None;
  std::bitset<FLAG_MAX> flags_;
  std::deque<Task*> dependencies_;

 public:
  explicit Task(const ArgsTypeBits& bits);
  virtual ~Task() = default;

  void exec(const Context* ctx);

  const ArgsTypeBits& type_bits() const { return type_bits_; }
  bool can_exec() const;
  bool is_done() const { return state_ == State::Done; }
  bool is_fence() const { return flags_.test(FLAG_FENCE); }

  bool add_dependency(Task* task);

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
  FuncTask(const FuncType& f) : Task(make_args_type_bits<As...>()), func_(f) {}

 protected:
  virtual void on_exec(const Context* ctx, TaskWork* work) override {
    func_(arg_traits<As>::to(ctx, work)...);
  }
};

}  // namespace sai::task