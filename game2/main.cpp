#define _SDL_main_h
#include <SDL.h>

#include <cassert>
#include <memory>
#include <array>
#include <thread>

// librl
#include "buffer2d.h"
#include "console.h"

// game
#include "prog.h"
#include "game.h"


int main(int argc, char *args[]) {

  game_t game;

  program_t prog{ game };
  if (!prog.init(320, 240, 2)) {
    return 1;
  }

  game.player_create();
  game.enemy_create();

  while (prog.active) {
    game.tick();

    prog.tick();
    prog.render();

    SDL_Delay(10);
    std::this_thread::yield();
  }

  return 0;
}
