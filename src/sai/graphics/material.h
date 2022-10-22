#pragma once
#include <SDL.h>

#include "color.h"

namespace sai::graphics {

// Material.
class Material {
 public:
  virtual ~Material() = default;
  virtual void render(SDL_Renderer* r, const SDL_Rect* rect) = 0;
};

// ColorMaterial.
class ColorMaterial : public Material {
 private:
  RGBA color_;

 public:
  explicit ColorMaterial(const RGBA& color) : color_(color) {}
  virtual void render(SDL_Renderer* r, const SDL_Rect* rect) override;
};

}  // namespace sai::graphics
