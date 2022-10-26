#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <SDL.h>

#include <algorithm>

#include "imgui.h"
#include "sai/core/frame.h"
#include "sai/debug/gui.h"
#include "sai/debug/performance.h"
#include "sai/ecs/registry.h"
#include "sai/graphics/sprite.h"
#include "sai/task/app.h"
#include "sai/task/scheduler.h"

namespace {

struct MovementComponent {
  int x;
  int y;
  int vx;
  int vy;
};

struct SpriteComponent {
  sai::graphics::SpriteHandle sprite_handle;
};

bool create_ball_entity(sai::ecs::Registry* registry,
                        sai::graphics::SpriteStorage* sprites) {
  auto id = registry->create_entity<MovementComponent, SpriteComponent>();

  if (auto mc = registry->get<MovementComponent>(id)) {
    mc->x = 0;
    mc->y = 0;
    mc->vx = 3;
    mc->vy = 3;
  }

  if (auto sc = registry->get<SpriteComponent>(id)) {
    SDL_Rect rect{0, 0, 10, 10};
    sai::graphics::RGBA color{0xff, 0xff, 0x00, 0xff};
    auto mat = std::make_unique<sai::graphics::ColorMaterial>(color);
    sc->sprite_handle =
        sprites->create(sai::graphics::Sprite{rect, std::move(mat)});
  }

  return true;
}

void update_movement(sai::ecs::Query<MovementComponent&> query,
                     const sai::video::RenderSize* size) {
  for (auto [mc] : query) {
    mc.x = std::min(mc.x, size->w);
    mc.y = std::min(mc.y, size->h);

    mc.x = mc.x + mc.vx;
    mc.y = mc.y + mc.vy;
    if (mc.x + mc.vx < 0 || size->w < mc.x + mc.vx) {
      mc.vx = -mc.vx;
    }
    if (mc.y + mc.vy < 0 || size->h < mc.y + mc.vy) {
      mc.vy = -mc.vy;
    }
  }
}

void update_sprites(
    sai::ecs::Query<const MovementComponent&, SpriteComponent&> query,
    sai::graphics::SpriteStorage* sprites) {
  for (auto [mc, sc] : query) {
    auto sprite = sprites->get(sc.sprite_handle);
    sprite->rect.x = mc.x - 5;
    sprite->rect.y = mc.y - 5;
  }
}

void render_debug_gui(sai::debug::Gui*, const sai::core::Frame* frame,
                      sai::task::Scheduler* scheduler,
                      sai::debug::PerformanceProfiler* profiler) {
  ImGui::Begin("Debug");
  sai::core::render_debug_gui(frame);
  if (ImGui::CollapsingHeader("Tasks")) {
    ImGui::Indent();
    scheduler->render_debug_gui();
    ImGui::Unindent();
  }
  profiler->render_debug_gui();
  ImGui::End();
}

}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
#if defined(_MSC_VER)
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  sai::task::App app;

  app.add_context<sai::ecs::Registry>();

  app.add_setup_task(create_ball_entity);

  app.add_task("update movecomponents", update_movement);
  app.add_task("apply sprites", update_sprites);
  app.add_task("render debug gui", render_debug_gui);

  return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
