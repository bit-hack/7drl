#pragma once

// librl
#include "random.h"

// game
#include "enums.h"

namespace game {

struct game_t;
struct entity_t;

struct map_generator_t {

  static const uint32_t num_colours = 3;

  map_generator_t(game::game_t &game)
    : game(game)
    , level(0)
  {
  }

  void generate(int32_t level);
  void place_rect();
  void place_walls();
  void place_items();
  void place_player();
  void place_grass();
  void drop_entity();
  void drop_enemy();

  void place_entity(entity_t *ent);

  int32_t rand(int32_t max) {
    return librl::random(seed) % max;
  }

  librl::int2 rand_map_coord();

  void mask_border();

  void fill_invalid(const librl::int2 &p);

  int32_t level;
  uint64_t seed;

  game_t &game;
};

}  // namespace game
