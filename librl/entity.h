#pragma once
#include <cstdint>

#include "gc.h"
#include "common.h"

namespace librl {

struct game_t;

struct entity_t : librl::gc_base_t {

  entity_t(const uint32_t type)
    : order(0)
    , pos(int2{-1, -1})
    , type(type)
  {
  }

  virtual void render(game_t &game) {
    // dummy
  }

  virtual void turn(game_t &game) = 0;

  template <typename type_t>
  bool is_a() const {
    return type_t::TYPE = type;
  }

  virtual void _enumerate(gc_enum_t &func) = 0;

  int2 pos;
  uint64_t order;

protected:
  const uint32_t type;
};

} // librl
