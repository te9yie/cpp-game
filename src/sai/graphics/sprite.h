#pragma once
#include <SDL.h>

#include <memory>

#include "../handle.h"
#include "../task/fwd.h"
#include "../video/video.h"
#include "material.h"

namespace sai::graphics {

// Sprite.
struct Sprite {
  SDL_Rect rect;
  Material material;
};

// SpriteStorage.
using SpriteHandle = Handle<Sprite>;
using SpriteStorage = HandleStorage<Sprite>;

void render_sprites(video::VideoSystem* sys, SpriteStorage* sprites);

void preset_graphics(task::App* app);

}  // namespace sai::graphics