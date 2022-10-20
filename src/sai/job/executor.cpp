#include "executor.h"

#include <algorithm>
#include <cstdio>

#include "job.h"

namespace sai::job {

/*explicit*/ Executor::Executor(std::string_view name) : name_(name) {}
/*virtual*/ Executor::~Executor() { stop(); }

bool Executor::start(std::size_t thread_n) {
  if (!threads_.empty()) return false;

  MutexPtr mutex(SDL_CreateMutex());
  if (!mutex) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[job::Executor::start] %s",
                 SDL_GetError());
    return false;
  }

  ConditionPtr condition(SDL_CreateCond());
  if (!condition) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[job::Executor::start] %s",
                 SDL_GetError());
    return false;
  }

  bool error = false;
  threads_.reserve(thread_n);
  for (std::size_t i = 0; i < thread_n; ++i) {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "%s_%zu", name_.c_str(), i);
    auto thread = SDL_CreateThread(Executor::thread_func_, buf, this);
    if (!thread) {
      SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[job::Executor::start] %s",
                   SDL_GetError());
      error = true;
      break;
    }
    threads_.emplace_back(thread);
  }
  if (error) {
    for (auto thread : threads_) {
      SDL_WaitThread(thread, nullptr);
    }
    threads_.clear();
    return false;
  }

  mutex_ = std::move(mutex);
  condition_ = std::move(condition);

  return true;
}
void Executor::stop() {
  if (threads_.empty()) return;

  is_stop_ = true;
  SDL_CondBroadcast(condition_.get());
  std::for_each(threads_.begin(), threads_.end(),
                [](auto t) { SDL_WaitThread(t, nullptr); });
  threads_.clear();

  mutex_.reset();
  condition_.reset();
  is_stop_ = false;
}

void Executor::submit(std::shared_ptr<Job> job) {
  UniqueLock lock(mutex_.get());
  job->set_observer(this);
  jobs_.emplace_back(std::move(job));
}
void Executor::kick() { SDL_CondBroadcast(condition_.get()); }

void Executor::join() {
  do {
    std::shared_ptr<Job> job;
    {
      UniqueLock lock(mutex_.get());
      bool is_empty = jobs_.empty() && active_job_count_ == 0;
      if (is_empty) break;
      auto it = std::find_if(jobs_.begin(), jobs_.end(),
                             [](auto& job) { return job->can_exec(); });
      if (it == jobs_.end()) {
        SDL_CondWait(condition_.get(), mutex_.get());
        continue;
      }
      job = *it;
      jobs_.erase(it);
      ++active_job_count_;
    }
    job->exec();
  } while (true);
}

/*virtual*/ void Executor::on_pre_exec_job(Job*) /*override*/ {}
/*virtual*/ void Executor::on_post_exec_job(Job* job) /*override*/ {
  {
    UniqueLock lock(mutex_.get());
    job->change_state(Job::State::Done);
    --active_job_count_;
  }
  SDL_CondBroadcast(condition_.get());
}

const char* Executor::get_thread_name_(SDL_threadID id) const {
  for (auto& t : threads_) {
    if (id == SDL_GetThreadID(t)) {
      return SDL_GetThreadName(t);
    }
  }
  return "";
}

void Executor::exec_jobs_() {
  while (!is_stop_) {
    std::shared_ptr<Job> job;
    {
      UniqueLock lock(mutex_.get());
      auto it = std::find_if(jobs_.begin(), jobs_.end(),
                             [](auto& job) { return job->can_exec(); });
      if (it == jobs_.end()) {
        SDL_CondWait(condition_.get(), mutex_.get());
        continue;
      }
      job = *it;
      jobs_.erase(it);
      ++active_job_count_;
    }
    job->exec();
  }
}

/*static*/ int Executor::thread_func_(void* p) {
  auto self = static_cast<Executor*>(p);
  self->exec_jobs_();
  return 0;
}

}  // namespace sai::job
