#define _SDL_main_h
#include <SDL.h>

#include <cassert>
#include <memory>
#include <array>

// librl
#include "buffer2d.h"
#include "console.h"
#include "game.h"

// game
#include "generator.h"
#include "enums.h"
#include "entities.h"

struct program_t {

  program_t(librl::game_t &game)
    : game(game)
    , width(0)
    , height(0)
    , active(false)
    , scale(0)
    , screen(nullptr)
  {
  }

  bool init(uint32_t w, uint32_t h, uint32_t s) {
    assert(screen == nullptr);
    assert(w && h);
    assert(s == 1 || s == 2);
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      return false;
    }
    screen = SDL_SetVideoMode(w * s, h * s, 32, 0);
    if (!screen) {
      return false;
    }
    active = true;
    width = w;
    height = h;
    scale = s;
    game.console_create(w / 8, h / 8);
    return true;
  }

  void on_event(const SDL_Event &event) {
    switch (event.type) {
    case SDL_QUIT:
      active = false;
      break;
    case SDL_KEYUP:

      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        active = false;
        break;
      case SDLK_SPACE:
        game.map_create(width / 8, height / 8);
        break;
      }

      if (!game.is_player_turn()) {
        return;
      }
      switch (event.key.keysym.sym) {
      case SDLK_LEFT:  game.input_event_push(librl::input_event_t{ librl::input_event_t::key_left  }); break;
      case SDLK_RIGHT: game.input_event_push(librl::input_event_t{ librl::input_event_t::key_right }); break;
      case SDLK_UP:    game.input_event_push(librl::input_event_t{ librl::input_event_t::key_up    }); break;
      case SDLK_DOWN:  game.input_event_push(librl::input_event_t{ librl::input_event_t::key_down  }); break;
      }
      break;
    case SDL_MOUSEBUTTONUP:
      break;
    }
  }

  void pump_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      on_event(event);
    }
  }

  void tick() {
    pump_events();
  }

  void render() {
    assert(screen);
    switch (scale) {
    case 1: render_x1(); break;
    case 2: render_x2(); break;
    default: assert(!"unsupported scale factor");
    }
  }

  void render_x1() {
    assert(screen);
    uint32_t *d0 = (uint32_t*)screen->pixels;
    const uint32_t pitch = width * 2;
    game.console_get().render(d0, pitch, width / 8, height / 8);
    SDL_Flip(screen);
  }

  void render_x2() {
    assert(screen);
    const uint32_t pitch = width * 2;
    uint32_t *d0 = (uint32_t*)screen->pixels;
    uint32_t *d1 = d0 + pitch;
    // render to every second scanline and in the left half of the screen
    game.console_get().render(d0, pitch * 2, width / 8, height / 8);
    // step over the screen unpacking the scanlines
    for (int y = 0; y < screen->h; y += 2) {
      for (int x = 0; x < screen->w; ++x) {
        d1[x] = d0[x >> 1];
      }
      // copy the now correct scanline to above one
      memcpy(d0, d1, screen->w * 4);
      // step down two scanlines
      d0 = d1 + pitch;
      d1 = d0 + pitch;
    }
    SDL_Flip(screen);
  }

  librl::game_t &game;

  uint32_t width, height;
  bool active;
  uint32_t scale;
  SDL_Surface *screen;
};


namespace game{

struct game_7drl_t : public librl::game_t {

  game_7drl_t() {
    generator.reset(new game::generator_2_t(*this));
  }

  void create_player() {
    assert(!player);
    player = gc.alloc<ent_player_t>(*this);
  }
};

} // namespace game

int main(int argc, char *args[]) {

  game::game_7drl_t game;
  game.create_player();
  program_t prog{ game };

  if (!prog.init(game::screen_width,
                 game::screen_height, 2)) {
    return 1;
  }

  game.map_create(prog.width / 8, prog.height / 8);

  while (prog.active) {
    prog.tick();
    prog.game.tick();
    prog.render();
    // dont burn up the CPU
    SDL_Delay(20);
  }

  return 0;
}
