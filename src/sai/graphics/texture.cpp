#include "texture.h"

#include "surface.h"

namespace sai::graphics {

// create_texture.
TexturePtr create_texture(SDL_Renderer* r, const std::uint8_t* data,
                          std::size_t size) {
  auto s = create_surface(data, size);
  if (!s) return nullptr;

  auto t = SDL_CreateTextureFromSurface(r, s.get());
  if (!t) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "error: %s", SDL_GetError());
    return nullptr;
  }

  return TexturePtr(t);
}

}  // namespace sai::graphics