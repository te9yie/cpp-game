#pragma once
#include <SDL.h>

#include <memory>

namespace sai::sync {

// ConditionPtr.
struct DestroyCondition {
  void operator()(SDL_cond* c) const { SDL_DestroyCond(c); }
};
using ConditionPtr = std::unique_ptr<SDL_cond, DestroyCondition>;

}  // namespace sai::sync