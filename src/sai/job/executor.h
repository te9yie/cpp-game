#pragma once
#include <SDL.h>

#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "job.h"
#include "sai/threading.h"
#include "t9/noncopyable.h"

namespace sai::job {

// Executor.
class Executor : t9::NonMovable {
 private:
  std::string name_;
  std::vector<SDL_Thread*> threads_;
  std::deque<std::shared_ptr<Job>> jobs_;
  MutexPtr mutex_;
  ConditionPtr condition_;
  volatile bool is_stop_ = false;

 public:
  explicit Executor(std::string_view name);
  virtual ~Executor();

  bool start(std::size_t thread_n);
  void stop();

  void submit(std::shared_ptr<Job> job);
  template <typename F>
  void submit_func(F f) {
    submit(std::make_shared<FuncJob>(f));
  }

 private:
  const char* get_thread_name_(SDL_threadID id) const;
  void exec_jobs_();

 private:
  static int thread_func_(void* p);
};

}  // namespace sai::job
