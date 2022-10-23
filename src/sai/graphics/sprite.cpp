#include "sprite.h"

#include "../task/app.h"

namespace sai::graphics {

void render_sprites(video::VideoSystem* sys, SpriteStorage* sprites) {
  auto r = sys->renderer.get();
  sprites->each([r](Sprite* s) { s->material->render(r, &s->rect); });
}

void preset_graphics(task::App* app) {
  app->add_context<SpriteStorage>();
  app->add_task_in_phase<task::RenderPhase>("render sprites", render_sprites);
}

}  // namespace sai::graphics