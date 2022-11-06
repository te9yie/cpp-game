#include "sprite.h"

#include "../task/app.h"
#include "surface.h"

namespace sai::graphics {

void render_sprite(SDL_Renderer* r, const asset::AssetStorage* assets,
                   const Sprite* s, const TextureStorage* ts) {
  const auto& m = s->material;
  SDL_SetRenderDrawColor(r, m.color.r, m.color.g, m.color.b, m.color.a);
  if (m.wireframe) {
    SDL_RenderDrawRect(r, &s->rect);
  } else {
    if (m.texture) {
      auto t = ts->get_mut(m.texture.id);
      if (!t->texture) {
        auto asset = assets->get(t->handle.id);
        if (asset->is_loaded) {
          auto surface = create_surface(asset->data.data(), asset->data.size());
          t->texture = create_texture(r, surface.get());
        }
      }
      if (t->texture) {
        const SDL_Rect* src = (m.texture_uv.w > 0 && m.texture_uv.h > 0)
                                  ? &m.texture_uv
                                  : nullptr;
        SDL_RenderCopy(r, t->texture.get(), src, &s->rect);
      }
    } else {
      SDL_RenderFillRect(r, &s->rect);
    }
  }
}

void render_sprites(video::VideoSystem* sys, const asset::AssetStorage* assets,
                    SpriteStorage* sprites, const TextureStorage* textures) {
  auto r = sys->renderer.get();
  sprites->each(
      [&](const Sprite* s) { render_sprite(r, assets, s, textures); });
}

}  // namespace sai::graphics