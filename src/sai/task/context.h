#pragma once
#include <cstdint>
#include <list>
#include <memory>
#include <unordered_map>

#include "t9/noncopyable.h"

namespace sai::task {

// Context.
class Context final : private t9::NonCopyable {
 private:
  // Storage.
  struct Storage {
    virtual ~Storage() = default;
  };
  template <typename T>
  struct TStorage : Storage {
    T x;
    template <typename... Args>
    TStorage(Args&&... args) : x(std::forward<Args>(args)...) {}
  };

  // type2index.
  template <typename T>
  struct type2index {
    static std::uintptr_t index() {
      static int i = 0;
      return reinterpret_cast<std::uintptr_t>(&i);
    }
  };

 public:
  std::list<std::unique_ptr<Storage>> storage_;
  std::unordered_map<std::uintptr_t, Storage*> index_;

 public:
  ~Context() { clear(); }

  template <typename T, typename... Args>
  T* add(Args&&... args) {
    auto i = type2index<T>::index();
    if (auto it = index_.find(i); it != index_.end()) return nullptr;
    auto s = std::make_unique<TStorage<T>>(std::forward<Args>(args)...);
    auto p = &s->x;
    index_.emplace(i, s.get());
    storage_.emplace_back(std::move(s));
    return p;
  }

  template <typename T>
  T* get() const {
    auto i = type2index<T>::index();
    auto it = index_.find(i);
    if (it == index_.end()) return nullptr;
    auto s = static_cast<TStorage<T>*>(it->second);
    return &s->x;
  }

  void clear() {
    while (!storage_.empty()) {
      storage_.pop_back();
    }
    index_.clear();
  }
};

}  // namespace sai::task