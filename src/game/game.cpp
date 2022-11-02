#include "game.h"

#include <algorithm>

#include "debug/manager.h"
#include "sai/ecs/registry.h"
#include "sai/graphics/sprite.h"
#include "sai/input/mouse.h"
#include "sai/task/app.h"

namespace {

template <typename T>
inline T rand_i(int min, int max) {
  return static_cast<T>(std::rand() % (max - min) + min);
}

struct CreateRect {};
struct ClearRects {};

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
                  sai::task::EventReader<CreateRect> create,
                  sai::task::EventReader<ClearRects> clear) {
  create.each([&](auto) {
    auto id = registry->create_entity<MovementComponent, SpriteComponent>();

    if (auto mc = registry->get<MovementComponent>(id)) {
      mc->x = rand_i<int>(0, size->w);
      mc->y = rand_i<int>(0, size->h);
      mc->vx = rand_i<int>(1, 3) * (rand_i<int>(0, 1) ? 1 : -1);
      mc->vy = rand_i<int>(1, 3) * (rand_i<int>(0, 1) ? 1 : -1);
    }

    if (auto sc = registry->get<SpriteComponent>(id)) {
      auto sprite = std::make_unique<sai::graphics::Sprite>();
      sprite->rect = SDL_Rect{0, 0, 10, 10};
      sprite->material.color =
          sai::graphics::Rgba{rand_i<Uint8>(0, 0xff), rand_i<Uint8>(0, 0xff),
                              rand_i<Uint8>(0, 0xff), 0xff};
      sc->handle = sprites->add(std::move(sprite));
    }
  });
  clear.each([&](auto) { registry->destroy_all_entities(); });
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

void render_debug_gui(sai::debug::Gui*, const sai::input::MouseState* mouse,
                      sai::task::EventWriter<CreateRect> create,
                      sai::task::EventWriter<ClearRects> clear) {
  ImGui::Begin("Debug");
  if (ImGui::Button("Create 1")) {
    create.notify(CreateRect{});
  }
  ImGui::SameLine();
  if (ImGui::Button("Create 100")) {
    for (int i = 0; i < 100; ++i) {
      create.notify(CreateRect{});
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Create 10000")) {
    for (int i = 0; i < 10000; ++i) {
      create.notify(CreateRect{});
    }
  }
  if (ImGui::Button("Clear")) {
    clear.notify(ClearRects{});
  }
  ImGui::Separator();
  ImGui::Text("mouse: %d, %d", mouse->x, mouse->y);

  ImGui::End();
}

}  // namespace

namespace game {

void preset_game(sai::task::App* app) {
  app->add_context<sai::ecs::Registry>();

  app->add_event<CreateRect>();
  app->add_event<ClearRects>();

  app->add_setup_task(setup_rects);

  app->add_task("create rects", create_rects);
  app->add_task("update movecomponents", update_movement);
  app->add_task("update sprites", update_sprites);
  app->add_task("render debug gui -main-", render_debug_gui);

  app->preset(debug::preset_debug);
}

}  // namespace game
