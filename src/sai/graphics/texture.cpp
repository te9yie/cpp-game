#include "texture.h"

#include "surface.h"

namespace sai::graphics {

// create_texture.
TexturePtr create_texture(SDL_Renderer* r, SDL_Surface* s) {
  auto t = SDL_CreateTextureFromSurface(r, s);
  if (!t) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "error: %s", SDL_GetError());
    return nullptr;
  }
  return TexturePtr(t);
}

}  // namespace sai::graphics