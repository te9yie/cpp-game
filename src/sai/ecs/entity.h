#pragma once
#include <cstddef>

namespace sai::ecs {

class Chunk;

// EntityId.
struct EntityId {
  std::size_t generation = 0;
  std::size_t index = 0;
};

// EntityEntry.
struct EntityEntry {
  std::size_t generation = 0;
  Chunk* chunk = nullptr;
  size_t chunk_index = 0;
};

// EntityStorage.
using EntityStorage = std::vector<EntityEntry>;

}  // namespace sai::ecs