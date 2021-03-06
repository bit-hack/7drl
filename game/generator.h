#pragma once
#include "game.h"

namespace game {

struct generator_1_t : public librl::map_generator_t {

  void generate(librl::game_t &game) override;
};

}  // namespace game
