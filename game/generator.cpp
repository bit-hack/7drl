// librl
#include "random.h"
#include "gc.h"
#include "common.h"
#include "perlin.h"
#include "bitset2d.h"

// game
#include "generator.h"
#include "entities.h"

namespace game {

void generator_2_t::generate(int32_t gen_level) {

  assert(game.player);
  game.entity_clear_all();
  game.entity_add(game.player);

  auto &map = game.map_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;

  level = gen_level;

  seed = game.random();

  game.map_get().clear(num_colours);

  for (int i = 0; i < 50; ++i) {
    place_rect();
  }
  place_walls();
  mask_border();
  place_player();
  // fill unreachable space so no items can be placed there
  fill_invalid(game.player->pos);

  place_items();
  place_grass();
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
  using namespace librl;
  auto &map = game.map_get();

  auto &walls = game.walls_get();
  walls.fill();

  const int32_t map_w = map.width;
  const int32_t map_h = map.height;
  for (int32_t y = 0; y < map_h-1; ++y) {
    for (int32_t x = 0; x < map_w-1; ++x) {
      auto &a = map.get(x, y + 0);
      auto &b = map.get(x, y + 1);
      auto &c = map.get(x + 1, y);

      if ((a != b || a != c) && (rand(4) > 0)) {
        a = tile_wall;
      }
      else {
        walls.clear(int2{ x, y });
        a = tile_floor;
      }
    }
  }
}

void generator_2_t::place_entity(librl::entity_t *e) {
  auto &map = game.map_get();
  for (int i = 0;; ++i) {
    if (i > 100) {
      return;  // oh noes
    }
    e->pos = rand_map_coord();
    if (map.get(e->pos.x, e->pos.y) == tile_floor) {
      break;
    }
  }
  game.entity_add(e);
}

void generator_2_t::drop_enemy() {
  using namespace librl;
  librl::entity_t *e = nullptr;

  int32_t l = clamp<int32_t>(0, int32_t(librl::random(seed, level / 2)), 6);

  switch (false ? 5 : l) {
  case 0: e = game.gc.alloc<game::ent_goblin_t>(game); break;
  case 1: e = game.gc.alloc<game::ent_vampire_t>(game); break;
  case 2: e = game.gc.alloc<game::ent_ogre_t>(game); break;
  case 3: e = game.gc.alloc<game::ent_wrath_t>(game); break;
  case 4: e = game.gc.alloc<game::ent_dwarf_t>(game); break;
  case 5: e = game.gc.alloc<game::ent_warlock_t>(game); break;
  case 6: e = game.gc.alloc<game::ent_mimic_t>(game); break;
  }
  assert(e);
  place_entity(e);
}

void generator_2_t::drop_entity() {
  using namespace librl;
  librl::entity_t *e = nullptr;

  const int32_t l = int32_t(librl::random(seed, min(level, 8)));
  switch (l) {
  case 0: e = game.gc.alloc<game::ent_club_t>(game); break;
  case 1: e = game.gc.alloc<game::ent_mace_t>(game); break;
  case 2: e = game.gc.alloc<game::ent_shield_t>(game); break;
  case 3: e = game.gc.alloc<game::ent_cloak_t>(game); break;
  case 4: e = game.gc.alloc<game::ent_leather_armour_t>(game); break;
  case 5: e = game.gc.alloc<game::ent_dagger_t>(game); break;
  case 6: e = game.gc.alloc<game::ent_metal_armour_t>(game); break;
  case 7: e = game.gc.alloc<game::ent_sword_t>(game); break;
  }
  assert(e);

  if (entity_equip_t *i = e->as_a<entity_equip_t>()) {

    if (i->damage) {
      i->damage += int(random(seed, level / 2));
    }
    else {
      i->defense += int(random(seed, level / 2));
    }

    i->crit     += int(random(seed, level / 2));
    i->evasion  += int(random(seed, level / 2));
    i->accuracy += int(random(seed, level / 2));
  }

  place_entity(e);
}

void generator_2_t::place_items() {
  using namespace librl;
  auto &map = game.map_get();
  for (int32_t i = 0; i < 6; ++i) {
    drop_enemy();
  }
  for (int32_t i = 0; i < 3; ++i) {
    entity_t *e = game.gc.alloc<game::ent_potion_t>(game);
    place_entity(e);
  }
  for (int32_t i = 0; i < 4; ++i) {
    entity_t *e = game.gc.alloc<game::ent_gold_t>(game);
    place_entity(e);
  }
  for (int32_t i = 0; i < 2; ++i) {
    drop_entity();
  }
  {
    entity_t *e = game.gc.alloc<game::ent_stairs_t>(game);
    place_entity(e);
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
  using namespace librl;
  auto &map = game.map_get();
  auto &walls = game.walls_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;
  for (int32_t y = 0; y < map_h; ++y) {
    map.get(0, y) = tile_wall;
    map.get(map_w - 1, y) = tile_wall;
    walls.set(int2{ 0, y });
    walls.set(int2{ map_w - 1, y });
  }
  for (int32_t x = 0; x < map_w; ++x) {
    map.get(x, 0) = tile_wall;
    map.get(x, map_h - 1) = tile_wall;
    walls.set(int2{ x, 0 });
    walls.set(int2{ x, map_h - 1 });
  }
}

void generator_2_t::place_grass() {
  librl::perlin_t perlin{ uint32_t(seed) };

  const uint32_t num_samples = 10;

  uint32_t norm = 0;
  for (uint32_t i = 0; i < num_samples; ++i) {
    const auto p = rand_map_coord();
    norm += perlin.sample2d(p.x, p.y);
  }
  norm /= num_samples;

  auto &map = game.map_get();
  const int32_t map_w = map.width;
  const int32_t map_h = map.height;
  for (int32_t y = 0; y < map_h; ++y) {
    for (int32_t x = 0; x < map_w; ++x) {
      uint8_t &c = map.get(x, y);
      if (c == tile_floor) {
        uint32_t v = librl::max(0, (int32_t(perlin.sample2d(x, y)) - int32_t(norm)));
        if (v > 64) {
          c = tile_grass;
        }
      }
    }
  }
}

void generator_2_t::fill_invalid(const librl::int2 &p) {
  using namespace librl;
  auto &map = game.map_get();
  auto &walls = game.walls_get();
  std::vector<int2> stack;
  bitset2d_t bits{ int(map.width), int(map.height) };
  bits.clear();
  // get current walls
  for (int y = 0; y < int(map.height); ++y) {
    for (int x = 0; x < int(map.width); ++x) {
      if (map.get(int2{ x, y }) == tile_wall) {
        bits.set(int2{ x, y });
      }
    }
  }
  // flood fill
  stack.push_back(p);
  while (!stack.empty()) {

    const int2 s = stack.back();
    stack.resize(stack.size() - 1);

    const int2 a = int2{ s.x - 1, s.y };
    if ((s.x > 0) && !bits.get(a)) {
      bits.set(a);
      stack.push_back(a);
    }
    const int2 b = int2{ s.x + 1, s.y };
    if ((s.x < int(map.width)-1) && !bits.get(b)) {
      bits.set(b);
      stack.push_back(b);
    }
    const int2 c = int2{ s.x, s.y - 1 };
    if ((s.y > 0) && !bits.get(c)) {
      bits.set(c);
      stack.push_back(c);
    }
    const int2 d = int2{ s.x, s.y + 1 };
    if ((s.x < int(map.height)-1) && !bits.get(d)) {
      bits.set(d);
      stack.push_back(d);
    }
  }
  // fill in walls
  for (int y = 0; y < int(map.height); ++y) {
    for (int x = 0; x < int(map.width); ++x) {
      if (!bits.get(int2{ x, y })) {
        map.get(int2{ x, y }) = tile_wall;
        walls.set(int2{ x, y });
      }
    }
  }
}

librl::int2 generator_2_t::rand_map_coord() {
  auto &map = game.map_get();
  return librl::int2{ rand(map.width), rand(map.height) };
}

}  // namespace game
