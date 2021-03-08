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

void game_t::render() {
  assert(map && console);
  auto &m = *map;
  auto &c = *console;

  if (!player) {
    return;
  }

  const int32_t px = player->pos.x;
  const int32_t py = player->pos.y;

  for (uint32_t y = 0; y < m.height; ++y) {
    for (uint32_t x = 0; x < m.width; ++x) {
      auto &cell = m.get(x, y);
#if 1
      // XXX: dont do this in the librl
      char ch = (cell == 0) ? '.' : '#';
      const int2 p = int2{ int32_t(x), int32_t(y) };

      const bool seen = raycast(player->pos, p, 1, m);
      if (seen) {
        fog->set(p);
      }
      else {
        if (!fog->get(p)) {
          ch = ' ';
        }
        else {
          ch = (cell == 0) ? ' ' : ch;
        }
      }
      c.chars.get(x, y) = ch;
#else
      c.chars.get(x, y) = '0' + pfield->read().get(x, y);
#endif
    }
  }

  // update entities
  for (auto &e : entities) {
    assert(e);
    e->render();
  }
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
  std::array<char, 1024> buf;
  va_list args;
  va_start(args, str);
  vsnprintf(buf.data(), buf.size(), str, args);
  va_end(args);
  buf.back() = '\0';
  printf("%s\n", buf.data());
}

}  // namespace librl
