#pragma once
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include "config.h"
#include "t9/noncopyable.h"
#include "threading.h"

namespace sai {

// Job.
class Job {
 public:
  virtual ~Job() = default;
  virtual void exec() = 0;
};

// FuncJob.
class FuncJob : public Job {
 private:
  std::function<void()> func_;

 public:
  template <typename F>
  FuncJob(F f) : func_(f) {}

  virtual void exec() override { func_(); }
};

// JobExecutor.
class JobExecutor : t9::NonMovable {
 private:
  std::string name_;
  std::vector<SDL_Thread*> threads_;
  std::deque<std::shared_ptr<Job>> jobs_;
  MutexPtr mutex_;
  ConditionPtr condition_;
  volatile bool is_stop_ = false;

 public:
  explicit JobExecutor(std::string_view name);
  virtual ~JobExecutor();

  bool start(std::size_t thread_n);
  void stop();

  void submit(std::shared_ptr<Job> job);
  template <typename F>
  void submit_func(F f) {
    submit(std::make_shared<FuncJob>(f));
  }

 private:
  void exec_jobs_();

 private:
  static int thread_func_(void* p);
};

}  // namespace sai
