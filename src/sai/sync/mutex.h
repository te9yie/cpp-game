#pragma once
#include <SDL.h>

#include <memory>

#include "t9/noncopyable.h"

namespace sai::sync {

// MutexPtr.
struct DestroyMutex {
  void operator()(SDL_mutex* m) const { SDL_DestroyMutex(m); }
};
using MutexPtr = std::unique_ptr<SDL_mutex, DestroyMutex>;

// UniqueLock.
class UniqueLock final : t9::NonCopyable {
 private:
  SDL_mutex* mutex_ = nullptr;

 public:
  explicit UniqueLock(SDL_mutex* m) : mutex_(m) { SDL_LockMutex(mutex_); }
  ~UniqueLock() { SDL_UnlockMutex(mutex_); }
};

}  // namespace sai::sync