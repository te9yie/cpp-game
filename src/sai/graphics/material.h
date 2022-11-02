#pragma once
#include <SDL.h>

#include "color.h"

namespace sai::graphics {

// Material.
struct Material {
  Rgba color;
  bool wireframe = false;
};

}  // namespace sai::graphics
