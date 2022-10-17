#include "ecs.h"

namespace game {

void create_entities(ecs::Registry* reg) {
  reg->create_entity<Status>();
  reg->create_entity<Status>();
}

void update_status(const Frame* frame,
                   ecs::Query<ecs::EntityId, Status&> query) {
  if (frame->frame_count % 60 == 0) {
    for (auto [id, status] : query) {
      status.hp += 1;
    }
  }
}
void show_status(ecs::Query<ecs::EntityId, const Status&> /*query*/) {}

}  // namespace game