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
    const auto order = ent->order;
    ent->turn();
    if (ent->order != order) {
      post_turn();
    }
  }
  // run a garbage collection cycle
  gc.collect();
}

void game_t::post_turn() {
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
      char ch = (cell== 0) ? '.' : '#';
      if (raycast(player->pos, int2{ int32_t(x), int32_t(y) }, 1, m)) {
        // mark tile to fow map
      }
      else {
        ch = ' ';
      }
      c.chars.get(x, y) = ch;
#else
      c.chars.get(x, y) = '0' + cell;
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
