#pragma once

namespace t9 {

// prevent unintended ADL.
namespace noncopyable_ {

// NonCopyable.
class NonCopyable {
 private:
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;

 protected:
  NonCopyable() = default;
  ~NonCopyable() = default;
};

// NonMovable.
class NonMovable {
 private:
  NonMovable(const NonMovable&) = delete;
  NonMovable& operator=(const NonMovable&) = delete;
  NonMovable(NonMovable&&) = delete;
  NonMovable& operator=(NonMovable&&) = delete;

 protected:
  NonMovable() = default;
  ~NonMovable() = default;
};

}  // namespace noncopyable_

using NonCopyable = noncopyable_::NonCopyable;
using NonMovable = noncopyable_::NonMovable;

}  // namespace t9
