#pragma once
#include <memory>
#include <vector>

#include "t9/noncopyable.h"

namespace sai::task {

// Context.
class Context final : private t9::NonCopyable {
 private:
  // EntryBase.
  struct EntryBase {
    virtual ~EntryBase() = default;
  };
  template <typename T>
  struct Entry : EntryBase {
    T x;
    template <typename... Args>
    Entry(Args&&... args) : x(std::forward<Args>(args)...) {}
  };

  // type2index.
  template <typename T>
  struct type2index {
    static inline std::size_t index = 0;
  };

 public:
  std::vector<std::unique_ptr<EntryBase>> storage_;

 public:
  ~Context() {
    for (auto it = storage_.rbegin(), last = storage_.rend(); it != last;
         ++it) {
      it->reset();
    }
  }

  template <typename T, typename... Args>
  T* add(Args&&... args) {
    auto i = type2index<T>::index;
    if (i != 0) return nullptr;
    auto e = std::make_unique<Entry<T>>(std::forward<Args>(args)...);
    auto p = &e->x;
    storage_.emplace_back(std::move(e));
    type2index<T>::index = storage_.size();
    return p;
  }

  template <typename T>
  T* get() const {
    auto i = type2index<T>::index;
    if (i == 0) return nullptr;
    auto e = static_cast<Entry<T>*>(storage_[i - 1].get());
    return &e->x;
  }
};

}  // namespace sai::task