#pragma once
#include <cassert>

#include "noncopyable.h"

namespace t9 {

// prevent unintended ADL.
namespace singleton_ {

// Singleton.
template <typename T>
class Singleton : private NonMovable {
 private:
  static inline T* instance_ = nullptr;

 protected:
  Singleton() {
    assert(!instance_);
    instance_ = static_cast<T*>(this);
  }
  ~Singleton() { instance_ = nullptr; }

 public:
  static T* instance() { return instance_; }
};

}  // namespace singleton_

template <typename T>
using Singleton = singleton_::Singleton<T>;

}  // namespace t9
