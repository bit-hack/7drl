#pragma once

#include "console.h"
#include "ecs.h"

struct game_t;

struct input_event_t {
  enum {
    key_up,
    key_down,
    key_left,
    key_right,
    key_enter,
    key_escape,
    key_i,  // inventory
    key_u,  // use
    key_d,  // drop
    key_e,  // equip
    mouse_lmb,
    mouse_rmb,
  } type;
  librl::int2 mouse_pos;
};

struct com_info_t {
  librl::int2 pos;
};

struct com_render_t  {
  void render(librl::ecs_id_t id, game_t &game);
  char glyph;
};

struct com_input_t {
  void recv(librl::ecs_id_t id, game_t &game, const input_event_t &event);
};

struct game_t {

  game_t()
    : player(librl::ecs_id_invalid) {
  }

  void player_create() {
    player = ecs.id_new();
    com_info_t *i = com_info.insert(player, new com_info_t);
    com_render_t *r = com_render.insert(player, new com_render_t);
    i->pos = librl::int2{ 0, 0 };
    r->glyph = '@';
  }

  void console_create(uint32_t w, uint32_t h) {
    _console.reset(new librl::console_t{ w, h });
  }

  void input_event_push(const input_event_t &event);

  librl::console_t &console() {
    return *_console;
  }

  librl::ecs_manager_t ecs;
  librl::ecs_store_t<com_info_t> com_info;
  librl::ecs_store_t<com_render_t> com_render;
  librl::ecs_store_t<com_input_t> com_input;

  void tick();

protected:
  librl::ecs_id_t player;

  std::unique_ptr<librl::console_t> _console;
};
