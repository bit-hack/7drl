#pragma
#include <cstdio>

// librl
#include "entity.h"
#include "gc.h"
#include "game.h"
#include "raycast.h"

// game
#include "enums.h"

namespace game {

struct ent_player_t : public librl::entity_actor_t {

  static const uint32_t TYPE = ent_type_player;

  ent_player_t(librl::game_t &game)
    : librl::entity_actor_t(TYPE, game)
  {
    name = "player";
    hp = 40;
  }

  virtual int32_t get_accuracy() const { return 50; }
  virtual int32_t get_damage() const   { return 10; }
  virtual int32_t get_defense() const  { return 0;  }
  virtual int32_t get_evasion() const  { return 10; }
  virtual int32_t get_crit() const     { return 2;  }

  void _enumerate(librl::gc_enum_t &func) override {
    // todo
  }

  void render() override {
    auto &con = game.console_get();
    con.chars.get(pos.x, pos.y) = '@';
  }

  void turn_end() {
    ++order;
    game.delay(500);
  }

  void interact_with(entity_t *e);

  void turn() override;
};

struct ent_test_t : public librl::entity_actor_t {

  static const uint32_t TYPE = ent_type_test;

  ent_test_t(librl::game_t &game)
    : librl::entity_actor_t(TYPE, game)
    , seed(game.random())
  {
    name = "goblin";
    hp = 30;
  }

  virtual int32_t get_accuracy() const { return 40; }
  virtual int32_t get_damage() const   { return 10; }
  virtual int32_t get_defense() const  { return 0;  }
  virtual int32_t get_evasion() const  { return 0;  }
  virtual int32_t get_crit() const     { return 0;  }

  void _enumerate(librl::gc_enum_t &func) override {
    // todo
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.chars.get(pos.x, pos.y) = 'H';
    }
  }

  void turn_end() {
    ++order;
    game.delay(500);
  }

  void interact_with(entity_t *e);

  void turn() override;

  uint64_t seed;
};

}  // game
