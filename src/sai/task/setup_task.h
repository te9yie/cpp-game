#pragma once
#include <functional>
#include <string>
#include <string_view>

#include "args.h"
#include "t9/noncopyable.h"

namespace sai::task {

class Context;

// SetupTask.
class SetupTask : private t9::NonCopyable {
 private:
  std::string name_;
  TaskWork work_;

 public:
  SetupTask(std::string_view name) : name_(name) {}
  virtual ~SetupTask() = default;

  bool exec(const Context* ctx) { return on_exec(ctx, &work_); }

  const std::string& name() const { return name_; }

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
  FuncSetupTask(std::string_view name, const FuncType& f)
      : SetupTask(name), func_(f) {}

 protected:
  virtual bool on_exec(const Context* ctx, TaskWork* work) override {
    return func_(arg_traits<As>::to(ctx, work)...);
  }
};

}  // namespace sai::task