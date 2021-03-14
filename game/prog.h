#pragma once
#define _SDL_main_h
#include <SDL.h>

#include <cassert>

#include "game.h"


struct program_t {

  program_t(game::game_t &game)
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

    SDL_WM_SetCaption("TinyRL", nullptr);

    SDL_EnableKeyRepeat(100, 100);

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
      //    case SDL_KEYUP:
    case SDL_KEYDOWN:

      switch (event.key.keysym.sym) {
      case SDLK_SPACE:
        // XXX: remove this!
//        game.map_next();
        break;
      }
      switch (event.key.keysym.sym) {
      case SDLK_LEFT:   game.input_event_push(game::input_event_t{ game::input_event_t::key_left }); break;
      case SDLK_RIGHT:  game.input_event_push(game::input_event_t{ game::input_event_t::key_right }); break;
      case SDLK_UP:     game.input_event_push(game::input_event_t{ game::input_event_t::key_up }); break;
      case SDLK_DOWN:   game.input_event_push(game::input_event_t{ game::input_event_t::key_down }); break;
      case SDLK_i:      game.input_event_push(game::input_event_t{ game::input_event_t::key_i }); break;
      case SDLK_u:      game.input_event_push(game::input_event_t{ game::input_event_t::key_u }); break;
      case SDLK_d:      game.input_event_push(game::input_event_t{ game::input_event_t::key_d }); break;
      case SDLK_e:      game.input_event_push(game::input_event_t{ game::input_event_t::key_e }); break;
      case SDLK_ESCAPE: game.input_event_push(game::input_event_t{ game::input_event_t::key_escape }); break;
      }
      break;
    case SDL_MOUSEBUTTONUP:
      game.input_event_push(game::input_event_t{
        (event.button.button == 0) ? game::input_event_t::mouse_lmb : game::input_event_t::mouse_rmb,
        event.button.x / 8,
        event.button.y / 8
        });
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
    const uint32_t pitch = width;
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

  game::game_t &game;

  uint32_t width, height;
  bool active;
  uint32_t scale;
  SDL_Surface *screen;
};
