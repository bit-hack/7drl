#pragma once
#include <cstdint>
#include <string>

#include "gc.h"
#include "common.h"

namespace librl {

struct game_t;

enum {
  subclass_actor,
  subclass_item,
};

struct entity_t : librl::gc_base_t {

  entity_t(const uint32_t type, const uint32_t sub, game_t &game)
    : order(0)
    , pos(int2{-1, -1})
    , type(type)
    , subclass(sub)
    , game(game)
  {
  }

  virtual void render() {
    // dummy
  }

  virtual bool turn() = 0;

  template <typename type_t>
  bool is_a() const {
    return type_t::TYPE = type;
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

  entity_actor_t(const uint32_t type, game_t &game)
    : entity_t(type, subclass_actor, game)
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

  entity_item_t(const uint32_t type, game_t &game)
    : entity_t(type, subclass_item, game)
  {
  }

  virtual void use_on(entity_t *e) = 0;
};

} // librl
