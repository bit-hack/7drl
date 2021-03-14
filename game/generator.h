#pragma once
// librl
#include "game.h"

// game
#include "enums.h"

namespace game {

struct generator_2_t : public game::map_generator_t {

  static const uint32_t num_colours = 3;

  generator_2_t(game::game_t &game)
    : game::map_generator_t(game)
    , level(0)
  {
  }

  void generate(int32_t level) override;
  void place_rect();
  void place_walls();
  void place_items();
  void place_player();
  void place_grass();
  void drop_entity();
  void drop_enemy();

  void place_entity(librl::entity_t *ent);

  int32_t rand(int32_t max) {
    return librl::random(seed) % max;
  }

  librl::int2 rand_map_coord() {
    auto &map = game.map_get();
    return librl::int2{rand(map.width), rand(map.height)};
  }

  void mask_border();

  void fill_invalid(const librl::int2 &p);

  int32_t level;
  uint64_t seed;
};

}  // namespace game
