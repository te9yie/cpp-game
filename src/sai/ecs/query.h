#pragma once
#include "../task/args.h"
#include "../task/context.h"
#include "chunk.h"
#include "entity.h"
#include "t9/func_traits.h"
#include "t9/type_list.h"
#include "type.h"

namespace sai::ecs {

// QueryIterator.
template <typename... Ts>
class QueryIterator {
 private:
  using AccessTuple = Chunk::AccessTuple<Ts...>;

 private:
  Chunk* chunk_ = nullptr;
  std::size_t chunk_index_ = 0;
  AccessTuple tuple_;

 public:
  QueryIterator() = default;
  QueryIterator(Chunk* chunk) : chunk_(chunk), chunk_index_(0) {
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
        tuple_ = chunk_->as_tuple<Ts...>(chunk_index_);
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
 private:
  using QueryType = t9::type_list<ecs_traits_t<Ts>...>;

 private:
  EntityStorage* entries_ = nullptr;
  Chunk* chunk_ = nullptr;

 public:
  explicit Query(EntityStorage* entries, Chunk* chunk) : entries_(entries) {
    for (auto it = chunk; it && !chunk_; it = it->next_chunk()) {
      if (it->contains<Ts...>()) {
        chunk_ = it;
      }
    }
  }

  template <typename T>
  T* get(EntityId id) const {
    static_assert(t9::contains_type<T, QueryType>::value,
                  "T is not included Query");
    if (id.index >= entries_->size()) return nullptr;
    if (id.generation != entries_->at(id.index).generation) return nullptr;
    auto& entry = entries_->at(id.index);
    return entry.chunk->get<T>(entry.chunk_index);
  }

  template <typename F>
  void each(F f) {
    if (!chunk_) return;
    each(f, t9::args_type<F>{});
  }
  template <typename F, typename... Ts>
  void each(F f, t9::type_list<Ts...> args) {
    for (auto chunk = chunk_; chunk; chunk = chunk->next_chunk()) {
      if (chunk->contains<Ts...>()) {
        chunk->each(f, args);
      }
    }
  }

  QueryIterator<Ts...> begin() const { return QueryIterator<Ts...>(chunk_); }
  QueryIterator<Ts...> end() const { return QueryIterator<Ts...>(); }
};

}  // namespace sai::ecs