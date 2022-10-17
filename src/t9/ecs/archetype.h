#pragma once
#include "chunk.h"
#include "config.h"
#include "t9/type_list.h"
#include "tuple.h"

namespace t9::ecs {

// Archetype.
class Archetype {
 private:
  std::unique_ptr<Tuple> tuple_;
  Chunk* for_iter_ = nullptr;

 public:
  Archetype(std::unique_ptr<Tuple> tuple) : tuple_(std::move(tuple)) {}

  void link_chunk(Chunk* chunk) {
    if (for_iter_) {
      for (auto it = for_iter_; it; it = it->next_same_archetype_chunk()) {
        if (it->next_same_archetype_chunk()) continue;
        it->link_same_archetype_chunk(chunk);
        break;
      }
    } else {
      for_iter_ = chunk;
    }
  }

  Chunk* get_free_chunk() {
    for (auto it = for_iter_; it; it = it->next_same_archetype_chunk()) {
      if (!it->is_full()) return it;
    }
    return nullptr;
  }

  const Tuple* tuple() const { return tuple_.get(); }

  template <typename... Ts>
  bool is_match() const {
    return tuple_->is_match(type_list<Ts...>{});
  }

 public:
  template <typename... Ts>
  static std::unique_ptr<Archetype> make() {
    auto tuple = Tuple::make<Ts...>();
    return std::make_unique<Archetype>(std::move(tuple));
  }
};

}  // namespace t9::ecs