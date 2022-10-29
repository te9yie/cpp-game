#if defined(_MSC_VER)
#include <crtdbg.h>
#endif

#include <SDL.h>

#include "game/game.h"
#include "sai/task/app.h"

int main(int /*argc*/, char* /*argv*/[]) {
#if defined(_MSC_VER)
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  sai::task::App app;
  app.preset(game::preset_game);
  return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
