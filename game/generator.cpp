// librl
#include "random.h"
#include "gc.h"
#include "common.h"

// game
#include "generator.h"
#include "entities.h"

namespace game {

void generator_2_t::generate() {

  assert(game.player);
  game.entity_clear_all();
  game.entity_add(game.player);
  game.player->order = 0;  // XXX: this is a hack (reset orders between levels)

  auto &map = game.map_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;

  seed = game.random();

  game.map_get().clear(num_colours);

  for (int i = 0; i < 50; ++i) {
    place_rect();
  }
  place_walls();
  mask_border();

  place_player();
  place_items();
}

void generator_2_t::place_rect() {
  auto &map = game.map_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;

  const int32_t x = rand(map_w);
  const int32_t y = rand(map_h);
  const int32_t w = 2 + rand(7);
  const int32_t h = 2 + rand(7);

  const int32_t x0 = librl::clamp(1, x - (w / 2),     map_w-1);
  const int32_t y0 = librl::clamp(1, y - (h / 2),     map_h-1);
  const int32_t x1 = librl::clamp(1, x - (w / 2) + w, map_w-1);
  const int32_t y1 = librl::clamp(1, y - (h / 2) + h, map_h-1);

  char v = rand(num_colours);

  for (int32_t y = y0; y < y1; ++y) {
    for (int32_t x = x0; x < x1; ++x) {
      map.get(x, y) = v;
    }
  }
}

void generator_2_t::place_walls() {
  auto &map = game.map_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;
  for (int32_t y = 0; y < map_h-1; ++y) {
    for (int32_t x = 0; x < map_w-1; ++x) {
      auto &a = map.get(x, y + 0);
      auto &b = map.get(x, y + 1);
      auto &c = map.get(x + 1, y);
      a = (a != b || a != c) && (rand(4) > 0) ? tile_wall : tile_floor;
    }
  }
}

void generator_2_t::place_items() {
  auto &map = game.map_get();
  for (int32_t i = 0; i < 6; ++i) {
    librl::entity_t *e = game.gc.alloc<game::ent_test_t>(game);
    for (;;) {
      e->pos = rand_map_coord();
      if (map.get(e->pos.x, e->pos.y) == tile_floor) {
        break;
      }
    }
    game.entity_add(e);
  }
}

void generator_2_t::place_player() {
  assert(game.player);
  librl::entity_t *e = game.player;
  auto &map = game.map_get();
  for (;;) {
    e->pos = rand_map_coord();
    if (map.get(e->pos.x, e->pos.y) == tile_floor) {
      break;
    }
  }
  game.player = game.entity_add(e);
}

void generator_2_t::mask_border() {
  auto &map = game.map_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;
  for (uint32_t y = 0; y < map_h; ++y) {
    map.get(0, y) = tile_wall;
    map.get(map_w - 1, y) = tile_wall;
  }
  for (uint32_t x = 0; x < map_w; ++x) {
    map.get(x, 0) = tile_wall;
    map.get(x, map_h - 1) = tile_wall;
  }
}

}  // namespace game
