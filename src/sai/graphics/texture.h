#pragma once
#include <SDL.h>

#include <memory>

#include "../handle.h"

namespace sai::graphics {

// TexturePtr.
struct DestroyTexture {
  void operator()(SDL_Texture* t) const { SDL_DestroyTexture(t); }
};
using TexturePtr = std::unique_ptr<SDL_Texture, DestroyTexture>;

// create_texture.
TexturePtr create_texture(SDL_Renderer* r, const std::uint8_t* data,
                          std::size_t size);

// Texture.
struct Texture {
  TexturePtr texture;
};

using TextureHandle = Handle<Texture>;
using TextureStorage = HandleStorage<Texture>;

}  // namespace sai::graphics