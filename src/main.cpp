#include <SDL.h>

#include "imgui.h"
#include "sai/core/frame.h"
#include "sai/debug/gui.h"
#include "sai/debug/performance.h"
#include "sai/graphics/sprite.h"
#include "sai/task/app.h"
#include "sai/task/phase.h"

namespace {

struct Ball {
  sai::graphics::SpriteHandle sprite_handle;
  int x = 0;
  int y = 0;
  int vx = 1;
  int vy = 1;
};

bool create_ball(sai::graphics::SpriteStorage* sprites, Ball* ball) {
  SDL_Rect rect{0, 0, 10, 10};
  sai::graphics::RGBA color{0xff, 0xff, 0x00, 0xff};
  auto mat = std::make_unique<sai::graphics::ColorMaterial>(color);
  ball->sprite_handle =
      sprites->create(sai::graphics::Sprite{rect, std::move(mat)});
  return true;
}

void update_ball(Ball* ball, const sai::graphics::SpriteStorage* sprites,
                 const sai::video::RenderSize* size) {
  auto sprite = sprites->get(ball->sprite_handle);
  ball->x = ball->x + ball->vx;
  ball->y = ball->y + ball->vy;
  if (ball->x + ball->vx < 0 || size->w < ball->x + ball->vx) {
    ball->vx = -ball->vx;
  }
  if (ball->y + ball->vy < 0 || size->h < ball->y + ball->vy) {
    ball->vy = -ball->vy;
  }
  sprite->rect.x = ball->x - 5;
  sprite->rect.y = ball->y - 5;
}

void render_debug_gui(sai::debug::Gui*, const sai::core::Frame* frame,
                      const sai::task::PhaseReference* phases,
                      const sai::debug::PerformanceProfiler* profiler) {
  ImGui::Begin("Debug");
  sai::core::render_debug_gui(frame);
  if (ImGui::CollapsingHeader("Tasks")) {
    ImGui::Indent();
    sai::task::render_debug_gui(phases);
    ImGui::Unindent();
  }
  profiler->render_debug_gui();
  ImGui::End();
}

}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
  sai::task::App app;

  app.add_context<sai::graphics::SpriteStorage>();
  app.add_context<Ball>();

  app.add_setup_task(create_ball);

  app.add_task("update ball", update_ball);
  app.add_task("render debug gui", render_debug_gui);
  app.add_task_in_phase<sai::task::RenderPhase>("render sprites",
                                                sai::graphics::render_sprites);

  return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
