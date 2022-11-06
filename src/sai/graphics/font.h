#pragma once
#include "../asset/asset.h"
#include "../video/video.h"
#include "texture.h"

namespace sai::graphics {

// Font.
struct Font {
  TextureHandle texture_handle;
};

// render_font.
void render_font(video::VideoSystem* sys, const asset::AssetStorage* assets,
                 const TextureStorage* textures, const Font* font, int x, int y,
                 const char* text);

}  // namespace sai::graphics