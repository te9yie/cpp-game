#pragma once
#include <functional>

#include "args.h"
#include "t9/noncopyable.h"

namespace sai::task {

class Context;

// SetupTask.
class SetupTask : private t9::NonCopyable {
 private:
  TaskWork work_;

 public:
  virtual ~SetupTask() = default;

  bool exec(const Context* ctx) { return on_exec(ctx, &work_); }

 protected:
  virtual bool on_exec(const Context* ctx, TaskWork* work) = 0;
};

// FuncSetupTask.
template <typename... As>
class FuncSetupTask : public SetupTask {
 public:
  using FuncType = std::function<bool(As...)>;

 private:
  FuncType func_;

 public:
  explicit FuncSetupTask(const FuncType& f) : func_(f) {}

 protected:
  virtual bool on_exec(const Context* ctx, TaskWork* work) override {
    return func_(arg_traits<As>::to(ctx, work)...);
  }
};

}  // namespace sai::task