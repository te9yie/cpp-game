#pragma once
#include "config.h"
#include "type.h"

namespace t9::ecs {

// Tuple.
class Tuple {
 private:
  // Element.
  struct Element {
    const Type* type = nullptr;
    std::size_t offset = 0;
  };

 private:
  std::unique_ptr<Element[]> types_;
  std::size_t type_count_ = 0;
  std::size_t memory_size_ = 0;

 public:
  template <std::size_t N>
  Tuple(const Type* (&types)[N])
      : types_(std::make_unique<Tuple::Element[]>(N)), type_count_(N) {
    auto fix_align = [](std::size_t offset, std::size_t align) {
      auto r = offset % align;
      return r == 0 ? offset : offset + align - r;
    };
    std::size_t offset = 0;
    std::size_t align = 0;
    for (std::size_t i = 0; i < N; ++i) {
      offset = fix_align(offset, types[i]->align);
      types_[i].type = types[i];
      types_[i].offset = offset;
      offset += types[i]->size;
      align = std::max(align, types[i]->align);
    }
    memory_size_ = fix_align(offset, align);
  }

  void construct(void* p) const {
    auto top = reinterpret_cast<std::uint8_t*>(p);
    for (std::size_t i = 0; i < type_count_; ++i) {
      types_[i].type->ctor(top + types_[i].offset);
    }
  }
  void destruct(void* p) const {
    auto top = reinterpret_cast<std::uint8_t*>(p);
    for (std::size_t i = 0; i < type_count_; ++i) {
      types_[i].type->dtor(top + types_[i].offset);
    }
  }

  std::size_t memory_size() const { return memory_size_; }

  template <typename T>
  bool try_get_offset(std::size_t* out_offset) const {
    assert(out_offset);
    auto type = Type::get<typename component_type_traits<T>::value_type>();
    for (std::size_t i = 0; i < type_count_; ++i) {
      if (types_[i].type == type) {
        *out_offset = types_[i].offset;
        return true;
      }
    }
    return false;
  }

  template <typename... Ts>
  bool is_match(type_list<Ts...>) const {
    std::size_t n = sizeof...(Ts);
    if (type_count_ != n) return false;
    const Type* types[] = {
        Type::get<typename component_type_traits<Ts>::value_type>()...};
    sort_types(types);
    for (std::size_t i = 0; i < n; ++i) {
      if (types_[i].type != types[i]) return false;
    }
    return false;
  }

  template <typename... Ts>
  bool contains(type_list<Ts...>) const {
    std::size_t n = sizeof...(Ts);
    if (type_count_ < n) return false;
    const Type* types[] = {
        Type::get<typename component_type_traits<Ts>::value_type>()...};
    sort_types(types);
    for (std::size_t i = 0, j = 0; i < type_count_; ++i) {
      if (types_[i].type == types[j]) {
        if (++j == n) return true;
      }
    }
    return false;
  }

  template <typename... Ts>
  static std::unique_ptr<Tuple> make() {
    const Type* types[] = {
        Type::get<typename component_type_traits<Ts>::value_type>()...};
    sort_types(types);
    return std::make_unique<Tuple>(types);
  }
};

}  // namespace t9::ecs