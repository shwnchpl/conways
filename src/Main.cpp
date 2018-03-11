
#include "Application.h"
#include "Config.h"

#include <memory>

// TODO: [ACTIVE] should display in title when game is active.
//       Escape should quit.
//       Command line arguments for speed and size would be nice.

int main(void)
{
  SDL_Event event;
  bool done = false, active = false;
  auto app = std::make_unique<Application>(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_WIDTH);

  app->Init();

  while (!done) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
          if (event.button.button == SDL_BUTTON_LEFT) {
            // XXX: This is a little ugly, but it gets the job done.
            app->ToggleCell(event.button.y / CELL_WIDTH, event.button.x / CELL_WIDTH); 
            app->UpdateView();
          }
          break;
        case SDL_KEYDOWN:
          if (event.key.keysym.sym == SDLK_SPACE) {
            active = !active;
          }
          break;
        case SDL_USEREVENT:
          if (active) {
            app->Iterate();
            app->UpdateView();
          }
          break;
        case SDL_QUIT:
          done = true;
          break;
        default:
          break;
      }
    }
  }

  app->Destroy();
  app.reset(nullptr);

  return 0;
}
