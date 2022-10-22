#include "material.h"

namespace sai::graphics {

/*virtual*/ void ColorMaterial::render(SDL_Renderer* r,
                                       const SDL_Rect* rect) /*override*/
{
  SDL_SetRenderDrawColor(r, color_.r, color_.g, color_.b, color_.a);
  SDL_RenderFillRect(r, rect);
}

}  // namespace sai::graphics