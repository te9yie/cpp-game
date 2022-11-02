#pragma once
#include <functional>

#include "noncopyable.h"

namespace t9 {

// Defer.
struct Defer final : NonCopyable {
  using FuncType = std::function<void(void)>;

  FuncType func;

  ~Defer() {
    if (func) {
      func();
    }
  }
};

}  // namespace t9