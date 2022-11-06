#include "game.h"

#include <algorithm>

#include "debug/manager.h"
#include "sai/asset/asset.h"
#include "sai/asset/manager.h"
#include "sai/graphics/sprite.h"
#include "sai/graphics/texture.h"
#include "sai/input/mouse.h"
#include "sai/task/app.h"

namespace {

template <typename T>
inline T rand_i(int min, int max) {
  return static_cast<T>(std::rand() % (max - min) + min);
}

constexpr int RECT_SIZE = 40;

struct RectEvent {
  enum class Type {
    Create,
    Destroy,
    Clear,
  };
  Type type;
  sai::ecs::EntityId id;
};

struct Font {
  sai::asset::AssetHandle handle;
  sai::graphics::SpriteHandle sprite;
};

struct Score {
  int score = 0;
};

struct MovementComponent {
  int x;
  int y;
  int vx;
  int vy;
};

struct SpriteComponent {
  sai::graphics::SpriteHandle handle;
};

bool setup_font(sai::asset::Manager* mgr, Font* font,
                sai::graphics::SpriteStorage* sprites,
                sai::graphics::TextureStorage* textures) {
  font->handle = mgr->load("assets/mplus_f12r.bmp");

  auto sprite = std::make_shared<sai::graphics::Sprite>();
  auto texture = std::make_shared<sai::graphics::Texture>();
  texture->handle = font->handle;
  sprite->rect = SDL_Rect{100, 100, 128, 128};
  sprite->material.texture_uv = SDL_Rect{0, 0, 128, 128};
  sprite->material.texture = textures->add(std::move(texture));
  font->sprite = sprites->add(std::move(sprite));

  return true;
}

bool setup_rects(sai::task::EventWriter<RectEvent> writer) {
  for (int i = 0; i < 10; ++i) {
    writer.notify(RectEvent{RectEvent::Type::Create});
  }
  return true;
}

void create_rects(sai::ecs::Registry* registry,
                  sai::graphics::SpriteStorage* sprites,
                  const sai::video::RenderSize* size,
                  sai::task::EventReader<RectEvent> events) {
  events.each([&](auto e) {
    switch (e.type) {
      case RectEvent::Type::Create: {
        auto id = registry->create_entity<MovementComponent, SpriteComponent>();

        if (auto mc = registry->get<MovementComponent>(id)) {
          mc->x = rand_i<int>(0, size->w);
          mc->y = rand_i<int>(0, size->h);
          mc->vx = rand_i<int>(1, 3) * (rand_i<int>(0, 1) ? 1 : -1);
          mc->vy = rand_i<int>(1, 3) * (rand_i<int>(0, 1) ? 1 : -1);
        }

        if (auto sc = registry->get<SpriteComponent>(id)) {
          auto sprite = std::make_shared<sai::graphics::Sprite>();
          sprite->rect = SDL_Rect{0, 0, RECT_SIZE, RECT_SIZE};
          sprite->material.color = sai::graphics::Rgba{
              rand_i<Uint8>(0, 0xff), rand_i<Uint8>(0, 0xff),
              rand_i<Uint8>(0, 0xff), 0xff};
          sc->handle = sprites->add(std::move(sprite));
        }
      } break;
      case RectEvent::Type::Destroy:
        registry->destroy_entity(e.id);
        break;
      case RectEvent::Type::Clear:
        registry->destroy_all_entities();
        break;
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

void click_check(
    const sai::input::MouseState* mouse,
    sai::ecs::Query<sai::ecs::EntityId, const MovementComponent&> query,
    Score* score, sai::task::EventWriter<RectEvent> writer) {
  SDL_Point p{mouse->x, mouse->y};
  if ((mouse->buttons & SDL_BUTTON_LMASK) != 0) {
    for (auto [id, mc] : query) {
      SDL_Rect rect{mc.x - RECT_SIZE / 2, mc.y - RECT_SIZE / 2, RECT_SIZE,
                    RECT_SIZE};
      if (SDL_PointInRect(&p, &rect)) {
        score->score += 100;
        writer.notify(RectEvent{RectEvent::Type::Destroy, id});
      }
    }
  }
}

void update_sprites(
    sai::ecs::Query<const MovementComponent&, SpriteComponent&> query,
    sai::graphics::SpriteStorage* sprites) {
  for (auto [mc, sc] : query) {
    auto sprite = sprites->get_mut(sc.handle.id);
    sprite->rect.x = mc.x - RECT_SIZE / 2;
    sprite->rect.y = mc.y - RECT_SIZE / 2;
  }
}

void render_debug_gui(sai::debug::Gui*, const sai::input::MouseState* mouse,
                      Score* score, sai::task::EventWriter<RectEvent> writer) {
  ImGui::Begin("Debug");
  if (ImGui::Button("Create 1")) {
    writer.notify(RectEvent{RectEvent::Type::Create});
  }
  ImGui::SameLine();
  if (ImGui::Button("Create 10")) {
    for (int i = 0; i < 10; ++i) {
      writer.notify(RectEvent{RectEvent::Type::Create});
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Create 100")) {
    for (int i = 0; i < 100; ++i) {
      writer.notify(RectEvent{RectEvent::Type::Create});
    }
  }
  if (ImGui::Button("Clear")) {
    writer.notify(RectEvent{RectEvent::Type::Clear});
  }

  ImGui::Separator();
  ImGui::Text("mouse: %d, %d", mouse->x, mouse->y);

  ImGui::Separator();
  if (ImGui::Button("Reset")) {
    score->score = 0;
  }
  ImGui::SameLine();
  ImGui::Text("Score: %d", score->score);

  ImGui::End();
}

}  // namespace

namespace game {

void preset_game(sai::task::App* app) {
  app->add_context<Font>();
  app->add_context<Score>();

  app->add_event<RectEvent>();

  app->add_setup_task(setup_font);
  app->add_setup_task(setup_rects);

  app->add_task("create rects", create_rects);
  app->add_task("update movecomponents", update_movement);
  app->add_task("update sprites", update_sprites);
  app->add_task("click check", click_check);
  app->add_task("render debug gui -main-", render_debug_gui);

  app->preset(debug::preset_debug);
}

}  // namespace game
