#pragma once
#include "chunk.h"
#include "config.h"
#include "entity.h"
#include "t9/function_traits.h"
#include "t9/type_list.h"

namespace t9::ecs {

// QueryIterator.
template <typename... Ts>
class QueryIterator {
 private:
  using AccessTuple = Chunk::AccessTuple<Ts...>;

 private:
  AccessTuple tuple_;
  Chunk* chunk_ = nullptr;
  std::size_t chunk_index_ = 0;

 public:
  QueryIterator() = default;
  explicit QueryIterator(Chunk* chunk) : chunk_(chunk), chunk_index_(0) {
    check_index();
  }

  QueryIterator& operator++() {
    ++chunk_index_;
    check_index();
    return *this;
  }

  AccessTuple* operator->() { return &tuple_; }
  AccessTuple& operator*() { return tuple_; }

  bool operator==(const QueryIterator& rhs) const {
    return chunk_ == rhs.chunk_ && chunk_index_ == rhs.chunk_index_;
  }
  bool operator!=(const QueryIterator& rhs) const { return !operator==(rhs); }

 private:
  bool check_index() {
    while (chunk_) {
      if (chunk_index_ >= chunk_->size()) {
        chunk_ = next_chunk();
        chunk_index_ = 0;
        if (!chunk_) return false;
      }
      if (chunk_->is_use(chunk_index_)) {
        tuple_ = chunk_->as_access_tuple<Ts...>(chunk_index_);
        return true;
      }
      ++chunk_index_;
    }
    return false;
  }
  Chunk* next_chunk() {
    auto chunk = chunk_;
    while (chunk) {
      chunk = chunk->next_chunk();
      if (!chunk) return nullptr;
      if (chunk->contains<Ts...>()) return chunk;
    }
    return nullptr;
  }
};

// Query.
template <typename... Ts>
class Query {
 public:
  using iterator = QueryIterator<Ts...>;

 private:
  EntityStorageVector* entities_ = nullptr;
  Chunk* chunk_ = nullptr;

 public:
  Query(EntityStorageVector* entities, Chunk* chunk) : entities_(entities) {
    for (auto it = chunk; it && !chunk_; it = it->next_chunk()) {
      if (it->contains<Ts...>()) {
        chunk_ = it;
      }
    }
  }

  template <typename F>
  void each(F f) {
    if (!chunk_) return;
    using args_type = typename function_traits<F>::args_type;
    each(f, args_type{});
  }
  template <typename F, typename... Ts>
  void each(F f, type_list<Ts...> args) {
    for (auto chunk = chunk_; chunk; chunk = chunk->next_chunk()) {
      if (chunk->contains<Ts...>()) {
        chunk->each(f, args);
      }
    }
  }

  iterator begin() const { return iterator(chunk_); }
  iterator end() const { return iterator(); }
};

}  // namespace t9::ecs