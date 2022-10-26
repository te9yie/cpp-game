#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <SDL.h>

#include <algorithm>
#include <cstdlib>

#include "imgui.h"
#include "sai/core/frame.h"
#include "sai/debug/gui.h"
#include "sai/debug/performance.h"
#include "sai/ecs/registry.h"
#include "sai/graphics/sprite.h"
#include "sai/task/app.h"
#include "sai/task/scheduler.h"

namespace {

template <typename T>
inline T rand_i(int min, int max) {
  return static_cast<T>(std::rand() % (max - min) + min);
}

struct CreateRect {};

struct MovementComponent {
  int x;
  int y;
  int vx;
  int vy;
};

struct SpriteComponent {
  sai::graphics::SpriteHandle handle;
};

bool setup_rects(sai::task::EventWriter<CreateRect> writer) {
  for (int i = 0; i < 10; ++i) {
    writer.notify(CreateRect{});
  }
  return true;
}

void create_rects(sai::ecs::Registry* registry,
                  sai::graphics::SpriteStorage* sprites,
                  const sai::video::RenderSize* size,
                  sai::task::EventReader<CreateRect> reader) {
  reader.each([&](auto) {
    auto id = registry->create_entity<MovementComponent, SpriteComponent>();

    if (auto mc = registry->get<MovementComponent>(id)) {
      mc->x = rand_i<int>(0, size->w);
      mc->y = rand_i<int>(0, size->h);
      mc->vx = rand_i<int>(1, 3) * (rand_i<int>(0, 1) ? 1 : -1);
      mc->vy = rand_i<int>(1, 3) * (rand_i<int>(0, 1) ? 1 : -1);
    }

    if (auto sc = registry->get<SpriteComponent>(id)) {
      SDL_Rect rect{0, 0, 10, 10};
      sai::graphics::RGBA color{rand_i<Uint8>(0, 0xff), rand_i<Uint8>(0, 0xff),
                                rand_i<Uint8>(0, 0xff), 0xff};
      auto mat = std::make_unique<sai::graphics::ColorMaterial>(color);
      sc->handle = sprites->create(sai::graphics::Sprite{rect, std::move(mat)});
    }
  });
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
    auto sprite = sprites->get(sc.handle);
    sprite->rect.x = mc.x - 5;
    sprite->rect.y = mc.y - 5;
  }
}

void render_debug_gui(sai::debug::Gui*, const sai::core::Frame* frame,
                      sai::task::Scheduler* scheduler,
                      sai::debug::PerformanceProfiler* profiler,
                      sai::task::EventWriter<CreateRect> writer) {
  ImGui::Begin("Debug");
  if (ImGui::Button("Create")) {
    writer.notify(CreateRect{});
  }
  ImGui::SameLine();
  if (ImGui::Button("Create 10")) {
    for (int i = 0; i < 10; ++i) {
      writer.notify(CreateRect{});
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Create 100")) {
    for (int i = 0; i < 100; ++i) {
      writer.notify(CreateRect{});
    }
  }
  ImGui::Separator();
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
  app.add_event<CreateRect>();

  app.add_setup_task(setup_rects);

  app.add_task("create rects", create_rects);
  app.add_task("update movecomponents", update_movement);
  app.add_task("apply sprites", update_sprites);
  app.add_task("render debug gui", render_debug_gui);

  return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}