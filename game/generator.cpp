// librl
#include "random.h"
#include "gc.h"
#include "common.h"

// game
#include "generator.h"
#include "entities.h"

namespace game {

void generator_1_t::generate(librl::game_t &game) {
  auto &map = game.map_get();
  const uint32_t map_w = map.width;
  const uint32_t map_h = map.height;

  // fill with random crap
  uint64_t seed = 12345;
  for (uint32_t y = 0; y < map.height; ++y) {
    for (uint32_t x = 0; x < map.width; ++x) {
      map.get(x, y) = 0;
    }
  }
  for (uint32_t i = 0; i < 50; ++i) {
    map.get(librl::random(seed) % map_w, librl::random(seed) % map_h) = 1;
  }

  // create a player
  librl::entity_t *e = game.gc.alloc<game::ent_player_t>();
  e->pos = librl::int2{4, 4};
  game.player = game.entity_add(e);

  // create enemies
  for (int i = 0; i < 4; ++i) {
    librl::entity_t *e = game.gc.alloc<game::ent_test_t>(game.random());
    e->pos = librl::int2{ librl::random(seed) % 16, librl::random(seed) % 16 };
    game.entity_add(e);
  }
}

}  // namespace game
