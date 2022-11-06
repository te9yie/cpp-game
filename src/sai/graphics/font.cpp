#include "font.h"

#include <cassert>

#include "surface.h"

namespace sai::graphics {

void render_font(video::VideoSystem* sys, const asset::AssetStorage* assets,
                 const TextureStorage* textures, const Font* font, int x, int y,
                 const char* text) {
  auto r = sys->renderer.get();
  auto texture = textures->get_mut(font->texture_handle.id);
  if (!texture->texture) {
    auto asset = assets->get(texture->handle.id);
    assert(asset);
    if (!asset->is_loaded) return;
    auto surface = create_surface(asset->data.data(), asset->data.size());
    assert(surface);
    auto t = create_texture(r, surface.get());
    texture->texture = std::move(t);
  }

  SDL_Rect dest{x, y, 8, 16};
  for (char c = *text++; c != '\0'; c = *text++) {
    int xi = c & 0x0f;
    int yi = (c & 0xf0) >> 4;
    SDL_Rect src{xi * 8, yi * 16, 8, 16};
    SDL_RenderCopy(r, texture->texture.get(), &src, &dest);
    dest.x += 8;
  }
}

}  // namespace sai::graphics
