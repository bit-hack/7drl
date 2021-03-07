#pragma once
// librl
#include "game.h"

// game
#include "enums.h"

namespace game {

struct generator_2_t : public librl::map_generator_t {

  static const uint32_t num_colours = 3;

  generator_2_t(librl::game_t &game)
    : librl::map_generator_t(game) {
  }

  void generate() override;
  void place_rect();
  void place_walls();
  void place_items();
  void place_player();

  int32_t rand(int32_t max) {
    return librl::random(seed) % max;
  }

  librl::int2 rand_map_coord() {
    auto &map = game.map_get();
    return librl::int2{rand(map.width), rand(map.height)};
  }

  void mask_border();

  uint64_t seed;
};

}  // namespace game
