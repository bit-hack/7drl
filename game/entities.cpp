#include "entities.h"

namespace game {

bool ent_player_t::turn() {
  const int32_t map_w = game.map_get().width;
  const int32_t map_h = game.map_get().height;
  librl::input_event_t event;
  if (!game.input_event_pop(event)) {
    return false;
  }

  librl::int2 np = pos;

  switch (event.type) {
  case librl::input_event_t::key_up:    np.y > 0         ? --np.y : 0; break;
  case librl::input_event_t::key_down:  np.y < map_h - 1 ? ++np.y : 0; break;
  case librl::input_event_t::key_left:  np.x > 0         ? --np.x : 0; break;
  case librl::input_event_t::key_right: np.x < map_w - 1 ? ++np.x : 0; break;
  }

  if (np == pos) {
    // no move selected
    return false;
  }

  // moving onto an entity which may count as an attack or pickup
  if (entity_t *ent = game.entity_find(np)) {
    interact_with(ent);
    return true;
  }

  // if moving to another floor tile
  if (game.map_get().get(np) == tile_floor) {
    pos = np;
    return true;
  }

  // not yet finished turn
  // ...
  return false;
}

void ent_player_t::interact_with(librl::entity_t *ent) {
  assert(ent);
  if (ent->is_type<ent_test_t>()) {
    attack(static_cast<librl::entity_actor_t*>(ent));
  }
  if (ent->is_subclass<librl::entity_item_t>()) {
    librl::entity_item_t *item = static_cast<librl::entity_item_t*>(ent);
    item->use_on(this);
  }
}

bool ent_test_t::turn() {
  const int32_t map_w = game.map_get().width;
  const int32_t map_h = game.map_get().height;

  librl::int2 np = pos;

#if 1
  int32_t dx = 0;
  int32_t dy = 0;
  game.pfield_get().diff(pos.x, pos.y, dx, dy);

  if (librl::abs(dx) > librl::abs(dy)) {
    np.x += dx;
  }
  else {
    np.y += dy;
  }
#else
  switch (librl::random(seed) & 3) {
  case 0: np.x < map_w - 1 ? ++np.x : 0; break;
  case 1: np.x > 0         ? --np.x : 0; break;
  case 2: np.y < map_h - 1 ? ++np.y : 0; break;
  case 3: np.y > 0         ? --np.y : 0; break;
  }
#endif

  if (entity_t *ent = game.entity_find(np)) {
    interact_with(ent);
    return true;
  }

  if (game.map_get().get(np) == tile_floor) {
    pos = np;
    return true;
  }

  // skip turn
  return true;
}

void ent_test_t::interact_with(entity_t *e) {
  switch (e->type) {
  case ent_type_player:
    attack(static_cast<entity_actor_t*>(e));
    break;
  }
}

}  // namespace game
