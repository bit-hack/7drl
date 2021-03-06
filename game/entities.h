#pragma
#include <cstdio>

#include "entity.h"
#include "gc.h"
#include "game.h"
#include "raycast.h"

namespace game {

enum {
  ent_type_none = 0,
  ent_type_player,
  ent_type_test,
};

struct ent_player_t : public librl::entity_t {

  static const uint32_t TYPE = ent_type_player;

  ent_player_t()
    : librl::entity_t(TYPE)
  {
  }

  void _enumerate(librl::gc_enum_t &func) override {
    // todo
  }

  void render(librl::game_t &game) override {
    auto &con = game.console_get();
    con.chars.get(pos.x, pos.y) = '@';
  }

  void turn(librl::game_t &game) override {
    const int32_t map_w = game.map_get().width;
    const int32_t map_h = game.map_get().height;
    librl::input_event_t event;
    if (game.input_event_pop(event)) {
      switch (event.type) {
      case librl::input_event_t::key_up:    pos.y > 0       ? --pos.y : 0; break;
      case librl::input_event_t::key_down:  pos.y < map_h-1 ? ++pos.y : 0; break;
      case librl::input_event_t::key_left:  pos.x > 0       ? --pos.x : 0; break;
      case librl::input_event_t::key_right: pos.x < map_w-1 ? ++pos.x : 0; break;
      }
      ++order;
      game.delay(500);
    }
  }
};

struct ent_test_t : public librl::entity_t {

  static const uint32_t TYPE = ent_type_player;
  uint64_t seed;

  ent_test_t(uint64_t seed)
    : librl::entity_t(TYPE)
    , seed(seed)
  {
  }

  void _enumerate(librl::gc_enum_t &func) override {
    // todo
  }

  void render(librl::game_t &game) override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.chars.get(pos.x, pos.y) = 'H';
    }
  }

  void turn(librl::game_t &game) override {
    const int32_t map_w = game.map_get().width;
    const int32_t map_h = game.map_get().height;
    switch (librl::random(seed) & 3) {
    case 0: pos.x < map_w-1 ? ++pos.x : 0; break;
    case 1: pos.x > 0       ? --pos.x : 0; break;
    case 2: pos.y < map_h-1 ? ++pos.y : 0; break;
    case 3: pos.y > 0       ? --pos.y : 0; break;
    }
    ++order;
    game.delay(500);
  }
};

}  // game
