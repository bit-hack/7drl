#include <stdarg.h>
#include <array>
#include <cstdio>

#include "game.h"
#include "raycast.h"
#include "perlin.h"

namespace librl {

void game_t::tick() {
  // update entities
  for (auto &e : entities) {
    assert(e);
    gc.check_in(e);
  }
  gc.check_in(player);

  // always tick the first entity
  if (!entities.empty()) {
    entity_t *ent = entities.front();
    assert(ent);
    if (ent->turn()) {
      post_turn();
    }
  }

  // if we have been instructed to generate a new map
  if (generate_new_map) {
    entities.clear();
    map_create(map->width, map->height);
    generate_new_map = false;
  }

  // run a garbage collection cycle
  gc.collect();
}

void game_t::post_turn() {
  // update the potential field
  assert(pfield);
  pfield->update();

#if 0
  // bubble entitiy down to new order slot
  auto itt = entities.begin();
  while (std::next(itt) != entities.end()) {
    if ((*itt)->order >= (*std::next(itt))->order) {
      std::swap(*itt, *std::next(itt));
      itt = std::next(itt);
    }
    else {
      break;
    }
  }
#else
  // move first to last
  std::rotate(entities.begin(), entities.begin() + 1, entities.end());
#endif

  // re-render the map
  render();
}

void game_t::render_entities() {
  for (auto &e : entities) {
    assert(e);
    e->render();
  }
}

void game_t::map_create(uint32_t w, uint32_t h) {
  // clear the old map entities entirely
  entities.clear();
  // create various maps and arrays
  fog.reset(new bitset2d_t(w, h));
  walls.reset(new bitset2d_t(w, h));
  map.reset(new buffer2d_u8_t(w, h));
  // run the map generator
  if (generator) {
    generator->generate();
  }
  // create a potential field
  assert(walls);
  pfield.reset(new pfield_t(*map, *walls));
  render();
}

void game_t::render() {
  if (!player) {
    return;
  }
  render_map();
  render_entities();
  render_hud();
}

entity_t *game_t::entity_find(const int2 &p) const {
  for (entity_t *e : entities) {
    if (e->pos == p) {
      return e;
    }
  }
  return nullptr;
}

void game_t::message_post(const char *str, ...) {
  assert(console);
  auto &c = *console;

  std::array<char, 1024> temp;
  va_list args;
  va_start(args, str);
  vsnprintf(temp.data(), temp.size(), str, args);
  va_end(args);
  temp.back() = '\0';

  c.fill(int2{ 0, c.height - 2 }, int2{ c.width, c.height - 1 }, ' ');
  c.caret_set(int2{ 0, c.height - 2 });
  c.puts(temp.data());
}

}  // namespace librl
