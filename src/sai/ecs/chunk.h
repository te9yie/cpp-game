#pragma once
#include <cassert>
#include <cstdint>
#include <vector>

#include "t9/noncopyable.h"
#include "t9/type_list.h"
#include "tuple.h"

namespace sai::ecs {

// Chunk.
class Chunk : private t9::NonCopyable {
 public:
  template <typename... Ts>
  struct AccessTuple {
    using tuple_type = std::tuple<Ts...>;
    template <std::size_t I>
    using element_type = std::tuple_element_t<I, tuple_type>;

    Chunk* chunk = nullptr;
    std::size_t index = 0;

    AccessTuple() = default;
    AccessTuple(Chunk* chunk, std::size_t index) : chunk(chunk), index(index) {}

    template <std::size_t I>
    element_type<I> get() {
      return *chunk->get<typename ecs_traits<element_type<I>>::type>(index);
    }
  };

 private:
  static const std::size_t BUFF_SIZE = 16 * 1024;

  struct IndexDesc {
    std::size_t next = 0;
    bool use = false;
  };

 private:
  const Tuple* tuple_ = nullptr;
  std::vector<IndexDesc> indices_;
  std::size_t next_index_ = 0;
  std::size_t peak_index_ = 0;
  Chunk* next_chunk_ = nullptr;
  Chunk* next_same_archetype_chunk_ = nullptr;
  std::uint8_t buff_[BUFF_SIZE] = {0};

 public:
  Chunk(const Tuple* tuple)
      : tuple_(tuple), indices_(BUFF_SIZE / tuple->memory_size()) {}

  std::size_t create() {
    assert(next_index_ < capacity());
    if (next_index_ >= peak_index_) {
      indices_[peak_index_].next = peak_index_ + 1;
      ++peak_index_;
    }
    auto index = next_index_;
    indices_[next_index_].use = true;
    next_index_ = indices_[next_index_].next;

    auto ptr = buff_ + tuple_->memory_size() * index;
    tuple_->construct(ptr);
    return index;
  }
  void destroy(std::size_t index) {
    auto ptr = buff_ + tuple_->memory_size() * index;
    tuple_->destruct(ptr);

    indices_[index].use = false;
    indices_[index].next = next_index_;
    next_index_ = index;
  }

  template <typename T>
  T* get(std::size_t index) {
    assert(is_use(index));
    std::size_t offset = 0;
    if (!tuple_->try_get_offset<T>(&offset)) return nullptr;
    auto ptr = buff_ + tuple_->memory_size() * index + offset;
    return reinterpret_cast<T*>(ptr);
  }

  template <typename... Ts>
  AccessTuple<Ts...> as_tuple(std::size_t index) {
    assert(is_use(index));
    return AccessTuple<Ts...>(this, index);
  }

  template <typename F, typename... Ts>
  void each(F f, t9::type_list<Ts...>) {
    for (std::size_t i = 0; i < peak_index_; ++i) {
      if (!indices_[i].use) continue;
      f(*get<sanitalize_t<Ts>>(i)...);
    }
  }

  std::size_t capacity() const { return indices_.size(); }
  std::size_t size() const { return peak_index_; }
  bool is_full() const { return next_index_ >= indices_.size(); }
  bool is_use(std::size_t i) const { return indices_[i].use; }

  template <typename... Ts>
  bool contains() {
    return tuple_->contains<Ts...>();
  }

  Chunk* next_chunk() { return next_chunk_; }
  Chunk* next_same_archetype_chunk() { return next_same_archetype_chunk_; }

  void link_chunk(Chunk* chunk) { next_chunk_ = chunk; }
  void link_same_archetype_chunk(Chunk* chunk) {
    next_same_archetype_chunk_ = chunk;
  }
};

}  // namespace sai::ecs

namespace std {

template <typename... Ts>
struct tuple_size<sai::ecs::Chunk::AccessTuple<Ts...>>
    : std::integral_constant<size_t, sizeof...(Ts)> {};

template <std::size_t I, typename... Ts>
struct tuple_element<I, sai::ecs::Chunk::AccessTuple<Ts...>> {
  using type = std::tuple_element_t<I, std::tuple<Ts...>>;
};

}  // namespace std