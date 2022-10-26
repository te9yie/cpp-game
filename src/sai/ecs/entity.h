#pragma once
#include <cstddef>

namespace sai::ecs {

class Chunk;

// EntityId.
struct EntityId {
  std::size_t generation = 0;
  std::size_t index = 0;
};

// EntityStorage.
struct EntityStorage {
  std::size_t generation = 0;
  Chunk* chunk = nullptr;
  size_t chunk_index = 0;
};

}  // namespace sai::ecs