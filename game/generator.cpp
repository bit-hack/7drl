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

void generator_2_t::generate(librl::game_t &game) {
  auto &map = game.map_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;

  seed = game.random();

  game.map_get().clear(num_colours);

  for (int i = 0; i < 50; ++i) {
    place_rect(game);
  }
  place_walls(game);

  // create a player
  librl::entity_t *e = game.gc.alloc<game::ent_player_t>();
  for (;;) {
    e->pos = librl::int2{ rand(map_w), rand(map_h) };
    if (map.get(e->pos.x, e->pos.y) == 0) {
      break;
    }
  }
  game.player = game.entity_add(e);

  place_items(game);
}

void generator_2_t::place_rect(librl::game_t &game) {
  auto &map = game.map_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;

  const int32_t x = rand(map_w);
  const int32_t y = rand(map_h);
  const int32_t w = 2 + rand(7);
  const int32_t h = 2 + rand(7);

  const int32_t x0 = librl::clamp(0, x - (w / 2),     map_w);
  const int32_t y0 = librl::clamp(0, y - (h / 2),     map_h);
  const int32_t x1 = librl::clamp(0, x - (w / 2) + w, map_w);
  const int32_t y1 = librl::clamp(0, y - (h / 2) + h, map_h);

  char v = rand(num_colours);

  for (int32_t y = y0; y < y1; ++y) {
    for (int32_t x = x0; x < x1; ++x) {
      map.get(x, y) = v;
    }
  }
}

void generator_2_t::place_walls(librl::game_t &game) {
  auto &map = game.map_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;
  for (int32_t y = 0; y < map_h-1; ++y) {
    for (int32_t x = 0; x < map_w-1; ++x) {
      auto &a = map.get(x, y + 0);
      auto &b = map.get(x, y + 1);
      auto &c = map.get(x + 1, y);
      a = (a != b || a != c) && (rand(4) > 0) ? 1 : 0;
    }
  }
}

void generator_2_t::place_items(librl::game_t &game) {
  auto &map = game.map_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;

  for (int32_t i = 0; i < 4; ++i) {

    librl::entity_t *e = game.gc.alloc<game::ent_test_t>(game.random());
    for (;;) {
      e->pos = librl::int2{ rand(map_w), rand(map_h) };
      if (map.get(e->pos.x, e->pos.y) == 0) {
        break;
      }
    }
    game.entity_add(e);

  }
}

}  // namespace game
