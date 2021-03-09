#include <stdarg.h>
#include <array>
#include <cstdio>

#include "game.h"
#include "raycast.h"

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
  if (player) {
    pfield->drop(player->pos.x, player->pos.y, 5);
  }
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

void game_t::render_map() {
  assert(map && console);
  auto &m = *map;
  auto &c = *console;

  const int32_t px = player->pos.x;
  const int32_t py = player->pos.y;

  static const std::array<char, 128> ramp = {
    '.', '#', '?'
  };

  for (uint32_t y = 0; y < m.height; ++y) {
    for (uint32_t x = 0; x < m.width; ++x) {
      auto &cell = m.get(x, y);

      uint8_t &ch = c.chars.get(x, y);

      ch = ramp[cell];
      const int2 p = int2{ int32_t(x), int32_t(y) };

      const bool seen = raycast(player->pos, p, 1, m);
      if (seen) {
        c.attrib.get(x, y) = 0xfac4d1;
        fog->set(int2{ int(x), int(y) });
      }
      else {
        c.attrib.get(x, y) = 0x404155;
        if (!fog->get(p)) {
          ch = ' ';
        }
      }
    }
  }
}

void game_t::render_entities() {
  for (auto &e : entities) {
    assert(e);
    e->render();
  }
}

void game_t::render_hud() {
  assert(console);
  auto &c = *console;

  assert(player);
  assert(player->is_subclass<entity_actor_t>());

  entity_actor_t *act = static_cast<entity_actor_t*>(player);

  console->fill(int2{0, c.height - 1}, int2{c.width, c.height}, ' ');

  console->caret_set(int2{ 0, c.height - 1 });
  console->print("level: %d  ", level);

  console->caret_set(int2{ 10, c.height - 1 });
  console->print("hp: %d  ", act->hp);
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
