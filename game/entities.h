#pragma once
#include <cstdio>
#include <deque>

// librl
#include "gc.h"
#include "raycast.h"

// game
#include "enums.h"
#include "inventory.h"


using namespace librl;

namespace game {

struct game_7drl_t;

enum subclass_t {
  ent_subclass_actor,
  ent_subclass_item,
  ent_subclass_equip,
};

struct entity_t : librl::gc_base_t {

  entity_t(const uint32_t type, const uint32_t subclass, game::game_7drl_t &game)
    : pos(librl::int2{ -1, -1 })
    , type(type)
    , subclass(subclass)
    , game(game)
  {
  }

  virtual void render() = 0;
  virtual bool turn() = 0;

  template <typename type_t>
  type_t* as_a() {
    return (subclass == type_t::SUBCLASS || type == type_t::TYPE)
      ? static_cast<type_t*>(this) : nullptr;
  }

  template <typename type_t>
  const type_t* as_a() const {
    return (subclass == type_t::SUBCLASS || type == type_t::TYPE)
      ? static_cast<const type_t*>(this) : nullptr;
  }

  template <typename type_t>
  bool is_type() const {
    return type_t::TYPE == type;
  }

  template <typename type_t>
  bool is_subclass() const {
    return type_t::SUBCLASS == subclass;
  }

  librl::int2 pos;
  const uint32_t type;
  const uint32_t subclass;
  std::string name;

protected:
  game::game_7drl_t &game;
};

struct entity_actor_t : public entity_t {

  static const subclass_t SUBCLASS = ent_subclass_actor;
  static const uint32_t TYPE = -1;

  entity_actor_t(const uint32_t type, game::game_7drl_t &game)
    : entity_t(type, SUBCLASS, game)
    , hp(0)
    , hp_max(0)
  {
  }

  virtual int32_t get_damage() const { return inventory.get_damage(); }
  virtual int32_t get_defense() const { return inventory.get_defense(); }
  virtual int32_t get_accuracy() const { return inventory.get_accuracy(); }
  virtual int32_t get_evasion() const { return inventory.get_evasion(); }
  virtual int32_t get_crit() const { return inventory.get_crit(); }

  virtual void attack(entity_actor_t *target);

  virtual void kill();

  virtual void on_take_damage(int32_t damage, entity_t *from) {};
  virtual void on_give_damage(int32_t damage, entity_t *to) {};

  int32_t hp;
  int32_t hp_max;
  inventory_t inventory;
};

struct entity_item_t : public entity_t {

  static const subclass_t SUBCLASS = ent_subclass_item;
  static const uint32_t TYPE = -1;

  entity_item_t(const uint32_t type, game::game_7drl_t &game, bool can_pickup)
    : entity_t(type, SUBCLASS, game)
    , can_pickup(can_pickup)
  {
  }

  bool turn() override {
    return true;
  }

  virtual void use_on(entity_t *e) = 0;

  virtual void picked_up(entity_t *by);

  void render() override;

  bool can_pickup;
  char glyph;
  uint32_t colour;
};

struct entity_equip_t : public entity_t {

  static const subclass_t SUBCLASS = ent_subclass_equip;
  static const uint32_t TYPE = -1;

  entity_equip_t(const uint32_t type, game::game_7drl_t &game)
    : entity_t(type, SUBCLASS, game)
    , damage(0)
    , accuracy(0)
    , evasion(0)
    , defense(0)
    , crit(0)
  {
    glyph = '?';
    colour = colour_item;
  }

  bool turn() override {
    return true;
  }

  virtual void picked_up(entity_t *by);

  void render() override;

  int32_t damage;
  int32_t accuracy;
  int32_t evasion;
  int32_t defense;
  int32_t crit;

  char glyph;
  uint32_t colour;
};

struct ent_player_t : public entity_actor_t {

  static const uint32_t TYPE = ent_type_player;

  ent_player_t(game_7drl_t &game);

  int32_t get_accuracy() const override { return entity_actor_t::get_accuracy() + 50; }
  int32_t get_damage() const   override { return entity_actor_t::get_damage() + 10; }
  int32_t get_defense() const  override { return entity_actor_t::get_defense() + 0;  }
  int32_t get_evasion() const  override { return entity_actor_t::get_evasion() + 10; }
  int32_t get_crit() const     override { return entity_actor_t::get_crit() + 2;  }

  void render() override;

  void kill() override;

  void interact_with(entity_t *e);

  bool turn() override;

  void _enumerate(librl::gc_enum_t &func) override;

  void on_give_damage(int32_t damage, entity_t *to) override;

  uint32_t gold;
  uint32_t xp;
  librl::int2 user_dir;
};

struct ent_enemy_t : public entity_actor_t {

  ent_enemy_t(uint32_t type, game::game_7drl_t &game);

  void render() override;

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

  uint8_t sense;

  uint32_t colour;
  char glyph;

  uint64_t seed;
};

struct ent_goblin_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_goblin;

  ent_goblin_t(game::game_7drl_t &game)
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

  ent_vampire_t(game::game_7drl_t &game)
    : ent_enemy_t(TYPE, game)
  {
    name = "vampire";
    hp = 50;
    accuracy = 75;
    damage = 15;
    glyph = 'v';
    colour = colour_vampire;
    sense = 245;
  }

  bool turn() override {
    hp += hp >= 50 ? 0 : 2;
    if (move_pfield(hp <= 15 ? -1 : 1)) {
      return true;
    }
    if (move_random()) {
      return true;
    }
    return true;
  }

  void on_give_damage(int32_t damage, entity_t *) override;
};

struct ent_ogre_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_ogre;

  ent_ogre_t(game::game_7drl_t &game)
    : ent_enemy_t(TYPE, game)
  {
    name = "ogre";
    hp = 170;
    accuracy = 75;
    damage = 16;
    glyph = 'o';
    colour = colour_ogre;
    sense = 245;
  }
};

struct ent_wrath_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_wrath;

  ent_wrath_t(game::game_7drl_t &game)
    : ent_enemy_t(TYPE, game)
    , timer(2)
  {
    name = "wrath";
    hp = 75;
    accuracy = 90;
    damage = 15;
    glyph = 'w';
    colour = colour_wrath;
    sense = 245;
  }

  void teleport_to_pos(const librl::int2 &p);

  bool turn() override;

  int timer;
};

struct ent_dwarf_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_dwarf;

  ent_dwarf_t(game::game_7drl_t &game)
    : ent_enemy_t(TYPE, game)
  {
    name = "dwarf";
    hp = 80;
    accuracy = 75;
    damage = 20;
    glyph = 'd';
    colour = colour_dwarf;
    sense = 248;
  }

  bool turn() override;
};

struct ent_warlock_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_warlock;

  static const uint32_t spawn_time = 10;

  ent_warlock_t(game::game_7drl_t &game)
    : ent_enemy_t(TYPE, game)
    , timer(spawn_time / 2)
  {
    name = "warlock";
    hp = 25;
    accuracy = 50;
    damage = 5;
    glyph = 'W';
    colour = colour_warlock;
    sense = 250;
  }

  void spawn_skeleton();

  bool turn() override;

  uint32_t timer;
};

struct ent_skeleton_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_mimic;

  ent_skeleton_t(game::game_7drl_t &game)
    : ent_enemy_t(TYPE, game)
  {
    name = "skeleton";
    hp = 80;
    accuracy = 75;
    damage = 20;
    glyph = 's';
    sense = 240;
    colour = colour_skeleton;
  }
};

struct ent_mimic_t : public ent_enemy_t {

  static const uint32_t TYPE = ent_type_mimic;

  ent_mimic_t(game::game_7drl_t &game)
    : ent_enemy_t(TYPE, game)
    , trigger(false)
  {
    name = "mimic";
    hp = 200;
    accuracy = 75;
    damage = 15;
    glyph = '?';
    colour = colour_item;
  }

  bool turn() override {
    if (trigger) {
      return ent_enemy_t::turn();
    }
    return true;
  }

  void on_take_damage(int32_t damage, entity_t *) override {
    trigger = true;
  }

  bool trigger;
};

struct ent_potion_t : public entity_item_t {

  static const uint32_t TYPE = ent_type_potion;

  ent_potion_t(game::game_7drl_t &game)
    : entity_item_t(TYPE, game, /* can_pickup */ true)
    , recovery(20)
  {
    name = "potion";
    glyph = 'p';
    colour = colour_potion;
  }

  void use_on(entity_t *e);

  const uint32_t recovery;
};

struct ent_stairs_t : public entity_item_t {

  // note: make sure to place where it wont get in the way of a corridor

  static const uint32_t TYPE = ent_type_stairs;

  ent_stairs_t(game::game_7drl_t &game)
    : entity_item_t(TYPE, game, /* can_pickup */ false)
    , seen(false)
  {
    name = "stairs";
    glyph = '=';
    colour = colour_stairs;
  }

  void use_on(entity_t *e);

  bool seen;
};

struct ent_gold_t : public entity_item_t {

  static const uint32_t TYPE = ent_type_gold;

  ent_gold_t(game::game_7drl_t &game);

  void use_on(entity_t *e);

  uint64_t seed;
};

struct ent_club_t : public entity_equip_t {

  static const uint32_t TYPE = ent_type_club;

  ent_club_t(game::game_7drl_t &game)
    : entity_equip_t(TYPE, game)
  {
    name = "club";
    damage = 4;
    evasion = -1;
  }
};

struct ent_mace_t : public entity_equip_t {

  static const uint32_t TYPE = ent_type_mace;

  ent_mace_t(game::game_7drl_t &game)
    : entity_equip_t(TYPE, game)
  {
    name = "mace";
    damage = 6;
    evasion = -1;
  }
};

struct ent_sword_t : public entity_equip_t {

  static const uint32_t TYPE = ent_type_sword;

  ent_sword_t(game::game_7drl_t &game)
    : entity_equip_t(TYPE, game)
  {
    name = "sword";
    damage = 8;
  }
};

struct ent_dagger_t : public entity_equip_t {

  static const uint32_t TYPE = ent_type_dagger;

  ent_dagger_t(game::game_7drl_t &game)
    : entity_equip_t(TYPE, game)
  {
    name = "dagger";
    damage = 6;
    evasion = 2;;
  }
};

struct ent_leather_armour_t : public entity_equip_t {

  static const uint32_t TYPE = ent_type_leather_armour;

  ent_leather_armour_t(game::game_7drl_t &game)
    : entity_equip_t(TYPE, game)
  {
    name = "leather armour";
    defense = 4;
  }
};

struct ent_shield_t : public entity_equip_t {

  static const uint32_t TYPE = ent_type_shield;

  ent_shield_t(game::game_7drl_t &game)
    : entity_equip_t(TYPE, game)
  {
    name = "shield";
    defense = 3;
    evasion = 2;
  }
};

struct ent_metal_armour_t : public entity_equip_t {

  static const uint32_t TYPE = ent_type_metal_armour;

  ent_metal_armour_t(game::game_7drl_t &game)
    : entity_equip_t(TYPE, game)
  {
    name = "leather armour";
    defense = 6;
    evasion = -1;
  }
};

struct ent_cloak_t : public entity_equip_t {

  static const uint32_t TYPE = ent_type_cloak;

  ent_cloak_t(game::game_7drl_t &game)
    : entity_equip_t(TYPE, game)
  {
    name = "cloak";
    defense = 3;
    evasion = 2;
  }
};

}  // game
