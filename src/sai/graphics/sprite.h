#pragma once
#include <SDL.h>

#include <memory>

#include "material.h"
#include "sai/video/video.h"
#include "t9/handle.h"

namespace sai::graphics {

// Sprite.
struct Sprite {
  SDL_Rect rect;
  std::unique_ptr<Material> material;
};

// SpriteStorage.
using SpriteHandle = t9::Handle<Sprite>;
using SpriteStorage = t9::HandleStorage<Sprite>;

void render_sprites(video::VideoSystem* sys, SpriteStorage* sprites);

}  // namespace sai::graphics