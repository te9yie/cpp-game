#pragma once
#include <memory>
#include <vector>

#include "t9/noncopyable.h"

namespace sai::task {

// Context.
template <typename Tag>
class Context final : private t9::NonCopyable {
 private:
  // TypeIndex.
  template <typename T>
  struct TypeIndex {
    static inline std::size_t index = 0;
  };

  // EntryBase.
  struct EntryBase {
   public:
    virtual ~EntryBase() = default;
  };

  // Entry.
  template <typename T>
  struct Entry : EntryBase {
    T x;
    template <typename... Args>
    Entry(Args&&... args) : x(std::forward<Args>(args)...) {}
  };

 private:
  std::vector<std::unique_ptr<EntryBase>> storage_;

 public:
  ~Context() {
    while (!storage_.empty()) {
      storage_.pop_back();
    }
  }

  template <typename T, typename... Args>
  T* add(Args&&... args) {
    auto i = TypeIndex<T>::index;
    if (i != 0) return nullptr;
    auto e = std::make_unique<Entry<T>>(std::forward<Args>(args)...);
    auto p = &e->x;
    storage_.emplace_back(std::move(e));
    TypeIndex<T>::index = storage_.size();
    return p;
  }

  template <typename T>
  T* get() const {
    auto i = TypeIndex<T>::index;
    if (i == 0) return nullptr;
    auto e = static_cast<Entry<T>*>(storage_[i - 1].get());
    return &e->x;
  }
};

using AppContext = Context<struct AppTag>;

}  // namespace sai::task
