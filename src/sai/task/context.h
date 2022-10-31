#pragma once
#include <memory>
#include <vector>

#include "t9/noncopyable.h"

namespace sai::task {

// ContextBase.
template <typename Tag>
class ContextBase final : private t9::NonCopyable {
 public:
  // type2index.
  template <typename T>
  struct type2index {
    static inline std::size_t index = 0;
  };

 private:
  // EntryBase.
  struct EntryBase {
   public:
    virtual ~EntryBase() = default;
  };
  template <typename T>
  struct Entry : EntryBase {
    T x;
    template <typename... Args>
    Entry(Args&&... args) : x(std::forward<Args>(args)...) {}
  };

 private:
  std::vector<std::unique_ptr<EntryBase>> storage_;

 public:
  ~ContextBase() {
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

using Context = ContextBase<struct AppTag>;

}  // namespace sai::task
