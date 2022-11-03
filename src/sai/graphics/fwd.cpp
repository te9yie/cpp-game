#include "fwd.h"

#include "../task/app.h"
#include "sprite.h"
#include "texture.h"

namespace sai::graphics {

void preset_graphics(task::App* app) {
  app->add_context<SpriteStorage>();
  app->add_context<TextureStorage>();
  app->add_task_in_phase<task::FirstPhase>(
      "update sprites", [](SpriteStorage* s) { s->update(); });
  app->add_task_in_phase<task::FirstPhase>(
      "update textures", [](TextureStorage* s) { s->update(); });
  app->add_task_in_phase<task::RenderPhase>("render sprites", render_sprites);
}

}  // namespace sai::graphics