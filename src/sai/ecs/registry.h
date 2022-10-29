#pragma once
#include <deque>
#include <memory>
#include <vector>

#include "archetype.h"
#include "chunk.h"
#include "entity.h"
#include "imgui.h"
#include "query.h"
#include "t9/func_traits.h"
#include "t9/noncopyable.h"

namespace sai::ecs {

// Registry.
class Registry : private t9::NonCopyable {
 private:
  std::vector<EntityStorage> entities_;
  std::deque<std::size_t> free_indices_;
  std::vector<std::unique_ptr<Archetype>> archetypes_;
  std::vector<std::unique_ptr<Chunk>> chunks_;
  Chunk* for_iter_ = nullptr;

 public:
  virtual ~Registry() { destroy_all_entities(); }

  template <typename... Ts>
  EntityId create_entity() {
    auto archetype = get_or_new_archetype<EntityId, Ts...>();
    auto chunk = get_or_new_chunk(archetype);
    auto index = create_entity_index();
    auto chunk_index = chunk->create();

    EntityId id = {entities_[index].generation, index};
    entities_[index].chunk = chunk;
    entities_[index].chunk_index = chunk_index;
    *chunk->template get<EntityId>(chunk_index) = id;

    return id;
  }

  bool destroy_entity(EntityId id) {
    if (id.index >= entities_.size()) return false;
    if (id.generation != entities_[id.index].generation) return false;

    auto& storage = entities_[id.index];
    storage.chunk->destroy(storage.chunk_index);

    free_indices_.push_back(id.index);
    storage.generation = std::max<size_t>(id.generation + 1, 1);
    storage.chunk = nullptr;
    storage.chunk_index = 0;

    return true;
  }

  void destroy_all_entities() {
    for (auto& entity : entities_) {
      if (!entity.chunk) continue;
      entity.chunk->destroy(entity.chunk_index);
    }
    entities_.clear();
    free_indices_.clear();
  }

  template <typename T>
  T* get(EntityId id) const {
    if (id.index >= entities_.size()) return nullptr;
    if (id.generation != entities_[id.index].generation) return nullptr;
    auto& storage = entities_[id.index];
    return storage.chunk->get<T>(storage.chunk_index);
  }

  template <typename... Ts>
  Query<Ts...> query() {
    return Query<Ts...>(for_iter_);
  }

 private:
  template <typename... Ts>
  Archetype* get_or_new_archetype() {
    for (auto& archetype : archetypes_) {
      if (archetype->is_match<Ts...>()) {
        return archetype.get();
      }
    }
    auto archetype = Archetype::make<Ts...>();
    auto p = archetype.get();
    archetypes_.emplace_back(std::move(archetype));
    return p;
  }

  Chunk* get_or_new_chunk(Archetype* archetype) {
    if (auto chunk = archetype->get_free_chunk()) {
      return chunk;
    }
    std::unique_ptr<Chunk> chunk(new Chunk(archetype->tuple()));
    auto p = chunk.get();
    chunks_.emplace_back(std::move(chunk));
    link_chunk(p);
    archetype->link_chunk(p);
    return p;
  }

  std::size_t create_entity_index() {
    size_t index = 0;
    if (free_indices_.empty()) {
      index = entities_.size();
      entities_.emplace_back().generation = 1;
    } else {
      index = free_indices_.front();
      free_indices_.pop_front();
    }
    return index;
  }

  void link_chunk(Chunk* chunk) {
    for (auto it = for_iter_; it; it = it->next_chunk()) {
      if (it->next_chunk()) continue;
      it->link_chunk(chunk);
      return;
    }
    for_iter_ = chunk;
  }

 public:
  void render_debug_gui() {
    auto entity_capacity = entities_.size();
    auto entity_count = entity_capacity - free_indices_.size();
    ImGui::Text("Entity: %zu/%zu", entity_count, entity_capacity);
    ImGui::Text("Archetype: %zu", archetypes_.size());
    ImGui::Text("Chunk: %zu", chunks_.size());
  }
};

}  // namespace sai::ecs

namespace sai::task {

namespace query_ {

template <typename T>
struct ecs_arg_traits {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_read<T>(); }
};

template <typename T>
struct ecs_arg_traits<T&> {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_write<T>(); }
};
template <typename T>
struct ecs_arg_traits<const T&> {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_read<T>(); }
};

}  // namespace query_

// arg_traits.
template <typename... Ts>
struct arg_traits<ecs::Query<Ts...>> {
  static void set_type_bits_(ArgsTypeBits*, t9::type_list<>) {}
  template <typename U, typename... Us>
  static void set_type_bits_(ArgsTypeBits* bits, t9::type_list<U, Us...>) {
    query_::ecs_arg_traits<U>::set_type_bits(bits);
    set_type_bits_(bits, t9::type_list<Us...>{});
  }

  static void set_type_bits(ArgsTypeBits* bits) {
    bits->set_read<ecs::Registry>();
    set_type_bits_(bits, t9::type_list<Ts...>{});
  }
  static ecs::Query<Ts...> to(const task::Context* ctx, task::TaskWork*) {
    return ctx->get<ecs::Registry>()->query<Ts...>();
  }
};

}  // namespace sai::task
