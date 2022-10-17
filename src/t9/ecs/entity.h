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

// EntityStorageVector.
using EntityStorageVector = std::vector<EntityStorage>;

}  // namespace t9::ecs