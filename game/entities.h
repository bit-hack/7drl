#pragma
#include "entity.h"
#include "gc.h"
#include "game.h"

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
    librl::input_event_t event;
    if (game.input_event_pop(event)) {
      switch (event.type) {
      case librl::input_event_t::key_up:
        --pos.y; ++order;
        break;
      case librl::input_event_t::key_down:
        ++pos.y; ++order;
        break;
      case librl::input_event_t::key_left:
        --pos.x; ++order;
        break;
      case librl::input_event_t::key_right:
        ++pos.x; ++order;
        break;
      }
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
    con.chars.get(pos.x, pos.y) = 'H';
  }

  void turn(librl::game_t &game) override {
    switch (librl::random(seed) & 3) {
    case 0: ++pos.x; break;
    case 1: --pos.x; break;
    case 2: ++pos.y; break;
    case 3: --pos.y; break;
    }
    ++order;
  }
};

}  // game
