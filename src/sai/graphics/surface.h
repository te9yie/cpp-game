#pragma once
#include <SDL.h>

#include <memory>

namespace sai::graphics {

// SurfacePtr.
struct DestroySurface {
  void operator()(SDL_Surface* s) const { SDL_FreeSurface(s); }
};
using SurfacePtr = std::unique_ptr<SDL_Surface, DestroySurface>;

// create_surface_from_data.
SurfacePtr create_surface(const std::uint8_t* data, std::size_t size);

}  // namespace sai::graphics