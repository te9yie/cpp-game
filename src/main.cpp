#include <SDL.h>

#include "imgui.h"
#include "sai/core/core.h"
#include "sai/core/frame.h"
#include "sai/debug/gui.h"
#include "sai/debug/performance.h"
#include "sai/graphics/sprite.h"
#include "sai/task/app.h"
#include "sai/task/executor.h"
#include "sai/video/video.h"

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

}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
  sai::task::App app;

  app.add_context<sai::debug::PerformanceProfiler>();
  app.add_context<sai::core::System>();
  app.add_context<sai::video::VideoSystem>();
  app.add_context<sai::debug::Gui>();
  app.add_context<sai::task::ExecutorWork>();
  app.add_context<sai::core::Frame>();
  app.add_context<sai::graphics::SpriteStorage>();
  app.add_context<sai::video::RenderSize>();
  app.add_context<Ball>();

  app.add_setup_task(sai::core::init_system);
  app.add_setup_task(sai::video::init_video_system);
  app.add_setup_task(sai::debug::init_gui_system);
  app.add_setup_task([](sai::debug::PerformanceProfiler* profiler) {
    profiler->setup_thread("MainThread");
    return true;
  });
  app.add_setup_task(create_ball);

  using Option = sai::task::TaskOption;

  app.add_task(
      "performance tick",
      [](sai::debug::PerformanceProfiler* profiler) { profiler->tick(); });
  app.add_task("frame tick", sai::core::tick_frame);
  app.add_task("handle events", sai::core::handle_events,
               Option().exclusive_this_thread().fence());
  app.add_task("-- begin debug gui", sai::debug::begin_frame, Option().fence());
  app.add_task("update ball", update_ball);
  app.add_task("render debug gui", [](sai::debug::Gui*,
                                      // sai::task::Executor* tasks,
                                      sai::debug::PerformanceProfiler* profiler,
                                      const sai::core::Frame* frame) {
    ImGui::Begin("Debug");

    auto delta = frame->start_count - frame->last_start_count;
    auto delta_ms = delta * 1000 / frame->frequency;
    ImGui::Text("delta (count): %lu", delta);
    ImGui::Text("delta (ms): %lu", delta_ms);
    ImGui::Text("frame count: %lu", frame->frame_count);
    ImGui::Text("delta: %f", frame->delta());

    ImGui::Separator();

    // tasks->render_debug_gui();
    profiler->render_debug_gui();
    ImGui::End();
  });
  app.add_task("-- end debug gui", sai::debug::end_frame, Option().fence());
  app.add_task("== begin render", sai::video::begin_render, Option().fence());
  app.add_task("render sprites", sai::graphics::render_sprites);
  app.add_task("render debug gui", sai::debug::render_gui);
  app.add_task("== end render", sai::video::end_render,
               Option().exclusive_this_thread().fence());

  return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
