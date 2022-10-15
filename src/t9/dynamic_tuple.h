#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>

#include "noncopyable.h"

namespace t9 {

// DynamicTuple.
class DynamicTuple : private NonMovable {
 private:
  template <typename T>
  struct type2id {
    static std::intptr_t value() {
      static int i = 0;
      return reinterpret_cast<std::intptr_t>(&i);
    }
  };

  // Storage.
  class Storage {
   public:
    virtual ~Storage() = default;
  };
  template <typename T>
  class StorageImpl : public Storage {
   private:
    std::unique_ptr<T> ptr_;

   public:
    template <typename... Args>
    StorageImpl(Args&&... args) : ptr_(new T(std::forward<Args>(args)...)) {}
    T* get() const { return ptr_.get(); }
  };

 private:
  std::map<std::intptr_t, std::unique_ptr<Storage>> storage_;

 public:
  DynamicTuple() = default;

  template <typename T, typename... Args>
  void set(Args&&... args) {
    std::unique_ptr<Storage> p =
        std::make_unique<StorageImpl<T>>(std::forward<Args>(args)...);
    storage_.emplace(type2id<T>::value(), std::move(p));
  }

  template <typename T>
  T* get() const {
    auto it = storage_.find(type2id<T>::value());
    if (it != storage_.end()) {
      auto storage = it->second.get();
      return static_cast<StorageImpl<T>*>(storage)->get();
    }
    return nullptr;
  }

  template <typename T>
  void remove() {
    auto it = storage_.find(type2id<T>::value());
    if (it != storage_.end()) {
      storage_.erase(it);
    }
  }
};

}  // namespace t9