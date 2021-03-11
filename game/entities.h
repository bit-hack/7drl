#pragma
#include <cstdio>
#include <deque>

// librl
#include "entity.h"
#include "gc.h"
#include "game.h"
#include "raycast.h"

// game
#include "enums.h"
#include "inventory.h"

namespace game {

struct ent_player_t : public librl::entity_actor_t {

  static const uint32_t TYPE = ent_type_player;

  ent_player_t(librl::game_t &game);

  int32_t get_accuracy() const override { return librl::entity_actor_t::get_accuracy() + 50; }
  int32_t get_damage() const   override { return librl::entity_actor_t::get_damage() + 10; }
  int32_t get_defense() const  override { return librl::entity_actor_t::get_defense() + 0;  }
  int32_t get_evasion() const  override { return librl::entity_actor_t::get_evasion() + 10; }
  int32_t get_crit() const     override { return librl::entity_actor_t::get_crit() + 2;  }

  void render() override {
    auto &con = game.console_get();
    con.attrib.get(pos.x, pos.y) = colour_player;
    con.chars.get(pos.x, pos.y) = '@';
  }

  void interact_with(entity_t *e);

  bool turn() override;

  void _enumerate(librl::gc_enum_t &func) override;

  uint32_t gold;
  librl::int2 user_dir;
};

struct ent_enemy_t : public librl::entity_actor_t {

  ent_enemy_t(uint32_t type, librl::game_t &game)
    : librl::entity_actor_t(type, game)
    , seed(game.random())
  {
    accuracy = 0;
    damage = 0;
    defense = 0;
    evasion = 0;
    crit = 0;
    colour = 0xFF00FF;
    glyph = '£';
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, game.walls_get())) {
      con.attrib.get(pos.x, pos.y) = colour;
      con.chars.get(pos.x, pos.y) = glyph;
    }
  }

  void interact_with(entity_t *e);

  bool turn() override;

  bool move_random();
  bool move_pfield(int dir = 1);

  int32_t get_accuracy() const override { return accuracy; }
  int32_t get_damage() const   override { return damage; }
  int32_t get_defense() const  override { return defense; }
  int32_t get_evasion() const  override { return evasion; }
  int32_t get_crit() const     override { return crit; }

  int32_t accuracy;
  int32_t damage;
  int32_t defense;
  int32_t evasion;
  int32_t crit;

  uint32_t colour;
  char glyph;

  uint64_t seed;
};

struct ent_goblin_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_goblin;

  ent_goblin_t(librl::game_t &game)
    : ent_enemy_t(TYPE, game)
  {
    name = "goblin";
    hp = 30;
    accuracy = 40;
    damage = 8;
    glyph = 'g';
    colour = colour_goblin;
  }
};

struct ent_vampire_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_vampire;

  ent_vampire_t(librl::game_t &game)
    : ent_enemy_t(TYPE, game)
  {
    name = "vampire";
    hp = 50;
    accuracy = 75;
    damage = 15;
    glyph = 'v';
    colour = colour_vampire;
  }

  bool turn() override {
    if (hp < 10) {
      // ... random chance to teleport away?
    }
    ++hp;
    return ent_enemy_t::turn();
  }

  void on_give_damage(int32_t damage) override {
    damage /= 2;
    game.message_post("%s leeched %d hp", name.c_str(), damage);
    hp += damage;
  }
};

struct ent_ogre_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_ogre;

  ent_ogre_t(librl::game_t &game)
    : ent_enemy_t(TYPE, game)
  {
    name = "ogre";
    hp = 170;
    accuracy = 75;
    damage = 16;
    glyph = 'o';
    colour = colour_ogre;
  }

  bool turn() override {

    // smash if too close

    return ent_enemy_t::turn();
  }
};

struct ent_wrath_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_wrath;

  ent_wrath_t(librl::game_t &game)
    : ent_enemy_t(TYPE, game)
    , timer(2)
  {
    name = "wrath";
    hp = 45;
    accuracy = 90;
    damage = 10;
    glyph = 'w';
    colour = colour_wrath;
  }

  void teleport_to_pos(const librl::int2 &p);

  bool turn() override {
    using namespace librl;
    if (game.player) {
      const auto &pp = game.player->pos;
      if (raycast(pp, pos, game.walls_get())) {
        --timer;
      }
      if (timer <= 0) {
        timer = 2;
        const int32_t dx = int32_t(pp.x) - int32_t(pos.x);
        const int32_t dy = int32_t(pp.y) - int32_t(pos.y);
        const int32_t dist = dx * dx + dy * dy;
        if (dist > 5) {
          teleport_to_pos(pp);
          return true;
        }
      }
    }
    return ent_enemy_t::turn();
  }

  int timer;
};

struct ent_dwarf_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_dwarf;

  ent_dwarf_t(librl::game_t &game)
    : ent_enemy_t(TYPE, game)
  {
    name = "dwarf";
    hp = 80;
    accuracy = 60;
    damage = 15;
    glyph = 'd';
    colour = colour_dwarf;
  }

  bool turn() override {
    using namespace librl;
    if (game.player) {
      // head straight for the player if close enough
      const auto &pp = game.player->pos;
      const int32_t dx = int32_t(pp.x) - int32_t(pos.x);
      const int32_t dy = int32_t(pp.y) - int32_t(pos.y);
      const int32_t dist = dx * dx + dy * dy;
      if (dist >= 1 && dist <= 8) {
        const int2 d = {
          abs(dx) > abs(dy) ? sign(dx) : 0,
          abs(dx) > abs(dy) ? 0 : sign(dy)
        };
        const int2 np{ pos.x + d.x, pos.y + d.y };
        // smash the wall down
        if (game.walls_get().get(np)) {
          game.walls_get().clear(np);
          game.map_get().get(np) = '.';
        }
        if (entity_t *e = game.entity_find(np)) {
          interact_with(e);
          return true;
        }
        else {
          pos = np;
          return true;
        }
      }
    }
    return ent_enemy_t::turn();
  }
};

struct ent_warlock_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_warlock;

  static const uint32_t spawn_time = 5;

  ent_warlock_t(librl::game_t &game)
    : ent_enemy_t(TYPE, game)
    , timer(spawn_time)
  {
    name = "warlock";
    hp = 25;
    accuracy = 50;
    damage = 5;
    glyph = 'W';
    colour = colour_warlock;
  }

  void spawn_skeleton();

  bool turn() {
    using namespace librl;
    if (!game.player) {
      return true;
    }
    const int2 &x = game.player->pos;
    if (raycast(x, pos, game.walls_get())) {
      if (timer-- == 0) {
        timer = spawn_time;
        spawn_skeleton();
      }
    }
    if (move_pfield(-1)) {
      return true;
    }
    if (move_random()) {
      return true;
    }
    // skip turn
    return true;
  }

  uint32_t timer;
};

struct ent_skeleton_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_mimic;

  ent_skeleton_t(librl::game_t &game)
    : ent_enemy_t(TYPE, game)
  {
    name = "skeleton";
    hp = 40;
    accuracy = 75;
    damage = 20;
    glyph = 's';
    colour = colour_skeleton;
  }
};

struct ent_mimic_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_mimic;

  static const uint32_t start_hp = 70;

  ent_mimic_t(librl::game_t &game)
    : ent_enemy_t(TYPE, game)
    , trigger(false)
  {
    name = "mimic";
    hp = start_hp;
    accuracy = 75;
    damage = 20;
    glyph = '?';
    colour = colour_item;
  }

  bool turn() override {
    if (trigger) {
      return ent_enemy_t::turn();
    }
    return true;
  }

  void on_take_damage(int32_t damage) override {
    trigger = true;
  }

  bool trigger;
};

struct ent_potion_t : public librl::entity_item_t {

  static const uint32_t TYPE = ent_type_potion;

  ent_potion_t(librl::game_t &game)
    : librl::entity_item_t(TYPE, game, /* can_pickup */ true)
    , recovery(20)
  {
    name = "potion";
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_potion;
      con.chars.get(pos.x, pos.y) = 'p';
    }
  }

  void use_on(entity_t *e) {
    using namespace librl;

    entity_actor_t *act = e->as_a<entity_actor_t>();
    assert(act);

    const int32_t before = act->hp;
    act->hp = min<int32_t>(act->hp_max, act->hp + recovery);
    const int32_t diff = act->hp - before;
    game.message_post("%s used %s to recover %u hp", e->name.c_str(),
        name.c_str(), diff);
  }

  const uint32_t recovery;
};

struct ent_stairs_t : public librl::entity_item_t {

  // note: make sure to place where it wont get in the way of a corridor

  static const uint32_t TYPE = ent_type_stairs;

  ent_stairs_t(librl::game_t &game)
    : librl::entity_item_t(TYPE, game, /* can_pickup */ false)
  {
    name = "stairs";
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, game.walls_get())) {
      con.attrib.get(pos.x, pos.y) = colour_stairs;
      con.chars.get(pos.x, pos.y) = '=';
    }
  }

  void use_on(entity_t *e) {
    if (e->is_type<ent_player_t>()) {
      game.message_post("%s proceeds deeper into the dungeon", e->name.c_str());
      game.map_next();
    }
  }
};

struct ent_gold_t : public librl::entity_item_t {

  static const uint32_t TYPE = ent_type_gold;

  ent_gold_t(librl::game_t &game)
    : librl::entity_item_t(TYPE, game, /* can_pickup */ false)
    , seed(game.random())
  {
    name = "gold";
  }

  void render() override {
    auto &con = game.console_get();
    if (game.player) {
      if (librl::raycast(game.player->pos, pos, game.walls_get())) {
        con.attrib.get(pos.x, pos.y) = colour_gold;
        con.chars.get(pos.x, pos.y) = '$';
      }
    }
  }

  void use_on(entity_t *e) {
    if (e->is_type<ent_player_t>()) {
      int value = int(15 + librl::random(seed, 15));
      game.message_post("%s gained %d gold", e->name.c_str(), value);
      static_cast<ent_player_t*>(e)->gold += value;
      game.entity_remove(this);
    }
  }

  uint64_t seed;
};

struct ent_club_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_club;

  ent_club_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "club";
    damage = 4;
    evasion = -1;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_mace_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_mace;

  ent_mace_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "mace";
    damage = 6;
    evasion = -1;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_sword_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_sword;

  ent_sword_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "sword";
    damage = 8;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_dagger_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_dagger;

  ent_dagger_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "dagger";
    damage = 6;
    evasion = 2;;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_leather_armour_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_leather_armour;

  ent_leather_armour_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "leather armour";
    defense = 3;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_metal_armour_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_leather_armour;

  ent_metal_armour_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "leather armour";
    defense = 6;
    evasion = -2;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_cloak_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_leather_armour;

  ent_cloak_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "cloak";
    defense = 1;
    evasion = 4;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

}  // game
