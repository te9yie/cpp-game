#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "t9/noncopyable.h"
#include "t9/type_list.h"

namespace sai::task {

// Context.
class Context final : private t9::NonCopyable {
 public:
  // type2index.
  template <typename T>
  struct type2index {
    static inline std::size_t index = 0;
  };

 private:
  // EntryBase.
  struct EntryBase {};
  template <typename T>
  struct Entry : EntryBase {
    T x;
    template <typename... Args>
    Entry(Args&&... args) : x(std::forward<Args>(args)...) {}
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

// ContextRef.
class ContextRef final {
 private:
  // type2index.
  template <typename T>
  using type2index = Context::type2index<T>;

 private:
  std::unordered_map<std::size_t, void*> storage_;

 public:
  template <typename... Ts>
  void setup(const Context* ctx) {
    insert_pair_<Ts...>(ctx, t9::type_list<Ts...>{});
  }

  template <typename T>
  T* get() const {
    auto i = type2index<T>::index;
    auto it = storage_.find(i);
    if (it == storage_.end()) return nullptr;
    return static_cast<T*>(it->second);
  }

 private:
  void insert_pair_(const Context*, t9::type_list<>) {}

  template <typename T, typename... Ts>
  void insert_pair_(const Context* ctx, t9::type_list<T, Ts...>) {
    storage_.emplace(type2index<T>::index, ctx->get<T>());
    insert_pair_(ctx, t9::type_list<Ts...>{});
  }
};
}  // namespace sai::task