#pragma once
#include <cstdint>
#include <string>

#include "gc.h"
#include "common.h"

namespace librl {

struct game_t;

enum subclass_t {
  ent_subclass_actor,
  ent_subclass_item,
};

struct entity_t : librl::gc_base_t {

  entity_t(const uint32_t type, const uint32_t subclass, game_t &game)
    : order(0)
    , pos(int2{-1, -1})
    , type(type)
    , subclass(subclass)
    , game(game)
  {
  }

  virtual void render() {
    // dummy
  }

  virtual bool turn() = 0;

  template <typename type_t>
  bool is_type() const {
    return type_t::TYPE == type;
  }

  template <typename type_t>
  bool is_subclass() const {
    return type_t::SUBCLASS == subclass;
  }

  virtual void _enumerate(gc_enum_t &func) = 0;

  uint64_t order;
  int2 pos;
  const uint32_t type;
  const uint32_t subclass;
  std::string name;

protected:
  game_t &game;
};

struct entity_actor_t : public entity_t {

  static const subclass_t SUBCLASS = ent_subclass_actor;

  entity_actor_t(const uint32_t type, game_t &game)
    : entity_t(type, SUBCLASS, game)
  {
  }

  virtual int32_t get_damage() const   { return 0; }
  virtual int32_t get_defense() const  { return 0; }
  virtual int32_t get_accuracy() const { return 0; }
  virtual int32_t get_evasion() const  { return 0; }
  virtual int32_t get_crit() const     { return 0; }

  virtual void attack(entity_actor_t *target);

  virtual void kill();

  int32_t hp;
};

struct entity_item_t : public entity_t {

  static const subclass_t SUBCLASS = ent_subclass_item;

  entity_item_t(const uint32_t type, game_t &game)
    : entity_t(type, SUBCLASS, game)
  {
  }

  virtual void use_on(entity_t *e) = 0;
};

} // librl
