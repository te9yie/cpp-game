#pragma once
#include <cassert>
#include <functional>

#include "noncopyable.h"

namespace t9 {

// Defer.
class Defer final : private NonCopyable {
 private:
  using FuncType = std::function<void(void)>;

 private:
  FuncType func_;

 public:
  Defer() = default;
  Defer(const FuncType& f) : func_(f) {}
  ~Defer() {
    if (func_) {
      func_();
    }
  }

  Defer& operator=(const FuncType& f) {
    assert(!func_);
    func_ = f;
    return *this;
  }
};

}  // namespace t9