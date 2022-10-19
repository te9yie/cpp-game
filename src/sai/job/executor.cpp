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
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[Executor::start] %s",
                 SDL_GetError());
    return false;
  }

  ConditionPtr condition(SDL_CreateCond());
  if (!condition) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[Executor::start] %s",
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
      SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[Executor::start] %s",
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
  jobs_.emplace_back(std::move(job));
  SDL_CondSignal(condition_.get());
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
      while (!is_stop_ && jobs_.empty()) {
        SDL_CondWait(condition_.get(), mutex_.get());
      }
      if (jobs_.empty()) continue;
      job = jobs_.front();
      jobs_.pop_front();
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
