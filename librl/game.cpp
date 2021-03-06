// librl
#include "game.h"

namespace librl {

void game_t::tick() {
  // render map
  map_render();
  // update entities
  for (auto &e : entities) {
    assert(e);
    e->render(*this);
    gc.check_in(e);
  }
  gc.check_in(player);
  // always tick the first entity
  if (!entities.empty()) {
    entity_t *ent = entities.front();
    assert(ent);
    const auto order = ent->order;
    entities.front()->turn(*this);
    // if the order was updated we need to sort the list
    if (order != ent->order) {
      entities_sort();
    }
  }
  // run a garbage collection cycle
  gc.collect();
}

void game_t::map_render() {
  assert(map && console);
  auto &m = *map;
  auto &c = *console;
  for (uint32_t y = 0; y < m.height; ++y) {
    for (uint32_t x = 0; x < m.width; ++x) {
      const auto &cell = m.get(x, y);
      c.chars.get(x, y) = cell;
    }
  }
}

void game_t::entities_sort() {
  struct {
    bool operator()(const entity_t *a, const entity_t *b) const {
      return a->order < b->order;
    }
  } compare;
  std::stable_sort(entities.begin(), entities.end(), compare);
}

}  // namespace librl
