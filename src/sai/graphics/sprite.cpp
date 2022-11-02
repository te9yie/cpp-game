#include "sprite.h"

#include "../task/app.h"

namespace sai::graphics {

void render_sprite(SDL_Renderer* r, const Sprite* s) {
  const auto& m = s->material;
  SDL_SetRenderDrawColor(r, m.color.r, m.color.g, m.color.b, m.color.a);
  if (m.wireframe) {
    SDL_RenderDrawRect(r, &s->rect);
  } else {
    SDL_RenderFillRect(r, &s->rect);
  }
}

void render_sprites(video::VideoSystem* sys, SpriteStorage* sprites) {
  sprites->update();
  auto r = sys->renderer.get();
  sprites->each([r](const Sprite* s) { render_sprite(r, s); });
}

void preset_graphics(task::App* app) {
  app->add_context<SpriteStorage>();
  app->add_task_in_phase<task::RenderPhase>("render sprites", render_sprites);
}

}  // namespace sai::graphics