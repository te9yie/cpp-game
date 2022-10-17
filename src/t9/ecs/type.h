#pragma once
#include "config.h"

namespace t9::ecs {

// Type.
struct Type {
  using Id = std::uintptr_t;
  using CtorFunc = void (*)(void*);
  using DtorFunc = void (*)(void*);

  template <typename T>
  struct type2id {
    static Id value() {
      static int i = 0;
      return reinterpret_cast<Id>(&i);
    }
  };

  Id id = 0;
  std::size_t size = 0;
  std::size_t align = 0;
  CtorFunc ctor = nullptr;
  DtorFunc dtor = nullptr;

  template <typename T>
  static const Type* get() {
    static const Type type{
        type2id<T>::value(),
        sizeof(T),
        alignof(T),
        [](void* p) { new (p) T; },
        [](void* p) { std::destroy_at(static_cast<T*>(p)); },
    };
    return &type;
  }
};

// sort_types.
template <typename... Ts, std::size_t N>
inline void sort_types(const Type* (&types)[N]) {
  std::sort(std::begin(types), std::end(types),
            [](const Type* lhs, const Type* rhs) {
              if (lhs->size < rhs->size) return false;
              if (lhs->size > rhs->size) return true;
              return lhs->id < rhs->id;
            });
}

// component_type_traits.
template <typename T>
struct component_type_traits {
  using value_type = T;
};

template <typename T>
struct component_type_traits<T*> {
  using value_type = T;
};

template <typename T>
struct component_type_traits<const T*> {
  using value_type = T;
};

template <typename T>
struct component_type_traits<T&> {
  using value_type = T;
};

template <typename T>
struct component_type_traits<const T&> {
  using value_type = T;
};

}  // namespace t9::ecs