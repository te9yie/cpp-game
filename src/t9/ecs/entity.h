#pragma once
#include "config.h"

namespace t9::ecs {

class Chunk;

// EntityId.
struct EntityId {
  std::size_t revision = 0;
  std::size_t index = 0;
};

// EntityStorage.
struct EntityStorage {
  std::size_t revision = 0;
  Chunk* chunk = nullptr;
  std::size_t chunk_index = 0;
};

class EntityContainer {
 private:
  std::vector<EntityStorage> entities_;
  std::deque<std::size_t> free_indices_;

 public:
  EntityId create() {
    auto index = new_index_();
    return EntityId{entities_[index].revision, index};
  }
  bool destroy(EntityId id) {
    if (id.index >= entities_.size()) return false;
    if (id.revision != entities_[id.index].revision) return false;
  }

 private:
  std::size_t new_index_() {
    std::size_t index = 0;
    if (free_indices_.empty()) {
      index = entities_.size();
      entities_.emplace_back().revision = 1;
    } else {
      index = free_indices_.front();
      free_indices_.pop_front();
    }
    return index;
  }
};

}  // namespace t9::ecs