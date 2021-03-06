#pragma once
#include "game.h"

namespace game {

struct generator_1_t : public librl::map_generator_t {
  void generate(librl::game_t &game) override;
};

struct generator_2_t : public librl::map_generator_t {

  static const uint32_t num_colours = 3;

  void generate(librl::game_t &game) override;

  void place_rect(librl::game_t &game);
  void place_walls(librl::game_t &game);
  void place_items(librl::game_t &game);

  int32_t rand(int32_t max) {
    return librl::random(seed) % max;
  }

  uint64_t seed;
};


}  // namespace game
