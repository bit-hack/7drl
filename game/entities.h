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
    hp = 100;
  }

  int32_t get_accuracy() const override { return 50; }
  int32_t get_damage() const   override { return 10; }
  int32_t get_defense() const  override { return 0;  }
  int32_t get_evasion() const  override { return 10; }
  int32_t get_crit() const     override { return 2;  }

  void _enumerate(librl::gc_enum_t &func) override {
    // todo
  }

  void render() override {
    auto &con = game.console_get();
    con.chars.get(pos.x, pos.y) = '@';
  }

  void interact_with(entity_t *e);

  bool turn() override;
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

  int32_t get_accuracy() const override { return 40; }
  int32_t get_damage() const   override { return 10; }
  int32_t get_defense() const  override { return 0;  }
  int32_t get_evasion() const  override { return 0;  }
  int32_t get_crit() const     override { return 0;  }

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

  void interact_with(entity_t *e);

  bool turn() override;

  uint64_t seed;
};

struct ent_potion_t : public librl::entity_item_t {

  static const uint32_t TYPE = ent_type_potion;

  ent_potion_t(librl::game_t &game)
    : librl::entity_item_t(TYPE, game)
    , recovery(20)
    , seed(game.random())
  {
    name = "potion";
  }

  void _enumerate(librl::gc_enum_t &func) override {
    // todo
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.chars.get(pos.x, pos.y) = 'p';
    }
  }

  void use_on(entity_t *e) {
    if (e->is_type<ent_player_t>()) {

      game.message_post("%s used %s to recovered %u health", e->name.c_str(),
          name.c_str(), recovery);

      static_cast<ent_player_t*>(e)->hp += recovery;
      game.entity_remove(this);
    }
  }

  bool turn() override {
    return true;
  }

  const uint32_t recovery;
  uint64_t seed;
};

}  // game
