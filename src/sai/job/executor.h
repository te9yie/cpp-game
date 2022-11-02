#pragma once
#include <SDL.h>

#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "../sync/condition.h"
#include "../sync/mutex.h"
#include "job.h"
#include "t9/noncopyable.h"

namespace sai::job {

// Executor.
class Executor : private t9::NonCopyable, public JobObserver {
 private:
  std::string name_;
  std::vector<SDL_Thread*> threads_;
  std::deque<std::shared_ptr<Job>> jobs_;
  std::size_t active_job_count_ = 0;
  sync::MutexPtr mutex_;
  sync::ConditionPtr condition_;
  volatile bool is_stop_ = false;

 public:
  explicit Executor(std::string_view name);
  virtual ~Executor();

  bool start(std::size_t thread_n);
  void stop();

  void submit(std::shared_ptr<Job> job);
  void kick();

  void join();

 public:
  // JobObserver.
  virtual void on_pre_exec_job(Job* job) override;
  virtual void on_post_exec_job(Job* job) override;

 private:
  const char* get_thread_name_(SDL_threadID id) const;
  void exec_jobs_();

 private:
  static int thread_func_(void* p);
};

}  // namespace sai::job
