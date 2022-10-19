#pragma once
#include <functional>

namespace sai::job {

// Job.
class Job {
 public:
  virtual ~Job() = default;
  virtual void exec() = 0;
};

// FuncJob.
class FuncJob : public Job {
 private:
  using FuncType = std::function<void(void)>;

 private:
  FuncType func_;

 public:
  FuncJob(const FuncType& f) : func_(f) {}

  virtual void exec() override { func_(); }
};

}  // namespace sai::job