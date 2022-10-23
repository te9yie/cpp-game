#pragma once
#include <SDL.h>

#include <memory>

#include "../task/fwd.h"
#include "../video/video.h"
#include "material.h"
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

void preset_graphics(task::App* app);

}  // namespace sai::graphics