#include <stdarg.h>
#include <array>
#include <cstdio>

#include "game.h"
#include "raycast.h"
#include "perlin.h"

namespace librl {

void game_t::tick_game() {
  // update entities
  for (auto &e : entities) {
    assert(e);
    gc.check_in(e);
  }
  if (player) {
    gc.check_in(player);
  }

  // tick the next entitys
  tick_entities();

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
  if (entities.empty() || !player) {
    return;
  }
  // tick the next entity
  tick_index++;
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
    generator->generate(level);
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

  c.attrib.fill(
    int2{ 0,       c.height - 5 },
    int2{ c.width, c.height - 1 },
    c.colour);
  c.window_set(
    int2{ 0,       c.height - 5 },
    int2{ c.width, c.height - 1 });
  c.caret_set(int2{ 0, c.height - 2 });
  c.window_scroll();
  c.fill(int2{ 0, c.height - 2 }, int2{ c.width, c.height - 1 }, ' ');
  c.puts(temp.data());
  c.window_reset();
}

}  // namespace librl
