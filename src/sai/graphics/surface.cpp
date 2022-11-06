#include "surface.h"

#include <SDL_image.h>

namespace sai::graphics {

// create_surface.
SurfacePtr create_surface(const std::uint8_t* data, std::size_t size) {
  auto rw =
      SDL_RWFromMem(const_cast<std::uint8_t*>(data), static_cast<int>(size));
  if (!rw) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "error: %s", SDL_GetError());
    return nullptr;
  }
  auto s = IMG_Load_RW(rw, SDL_TRUE);
  if (!rw) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "error: %s", SDL_GetError());
    return nullptr;
  }
  return SurfacePtr(s);
}

}  // namespace sai::graphics
