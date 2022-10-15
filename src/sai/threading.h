#pragma once
#include <memory>

#include "config.h"

namespace sai {

// MutexPtr.
struct DestroyMutex {
  void operator()(SDL_mutex* m) const { SDL_DestroyMutex(m); }
};
using MutexPtr = std::unique_ptr<SDL_mutex, DestroyMutex>;

// ConditionPtr.
struct DestroyCondition {
  void operator()(SDL_cond* c) const { SDL_DestroyCond(c); }
};
using ConditionPtr = std::unique_ptr<SDL_cond, DestroyCondition>;

// UniqueLock.
class UniqueLock final : t9::NonMovable {
 private:
  SDL_mutex* mutex_ = nullptr;

 public:
  UniqueLock(SDL_mutex* m) : mutex_(m) { SDL_LockMutex(mutex_); }
  ~UniqueLock() { SDL_UnlockMutex(mutex_); }
};

}  // namespace sai
