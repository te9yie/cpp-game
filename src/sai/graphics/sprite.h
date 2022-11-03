#pragma once
#include <SDL.h>

#include <memory>

#include "../handle.h"
#include "../task/fwd.h"
#include "../video/video.h"
#include "material.h"
#include "texture.h"

namespace sai::graphics {

// Sprite.
struct Sprite {
  SDL_Rect rect;
  Material material;
};

// SpriteStorage.
using SpriteHandle = Handle<Sprite>;
using SpriteStorage = HandleStorage<Sprite>;

void render_sprites(video::VideoSystem* sys, const SpriteStorage* sprites,
                    const TextureStorage* textures);

}  // namespace sai::graphics