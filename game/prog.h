#pragma once
#define _SDL_main_h
#include <SDL.h>

#include <cassert>

#include "game.h"


struct program_t {

  program_t(game::game_7drl_t &game)
    : game(game)
    , width(0)
    , height(0)
    , active(false)
    , scale(0)
    , screen(nullptr)
  {
  }

  bool init(uint32_t w, uint32_t h, uint32_t s);

  void on_event(const SDL_Event &event);

  void pump_events();

  void tick();

  void render();

  void render_x1();

  void render_x2();

  game::game_7drl_t &game;

  uint32_t width, height;
  bool active;
  uint32_t scale;
  SDL_Surface *screen;
};
