#pragma once
#include <SDL.h>

#include "color.h"
#include "texture.h"

namespace sai::graphics {

// Material.
struct Material {
  Rgba color;
  bool wireframe = false;
  TextureHandle texture;
  SDL_Rect texture_uv;
};

}  // namespace sai::graphics
