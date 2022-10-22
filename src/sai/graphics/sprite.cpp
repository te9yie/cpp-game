#include "sprite.h"

namespace sai::graphics {

void render_sprites(video::VideoSystem* sys, SpriteStorage* sprites) {
  auto r = sys->renderer.get();
  sprites->each([r](Sprite* s) { s->material->render(r, &s->rect); });
}

}  // namespace sai::graphics