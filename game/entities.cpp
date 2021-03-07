#include "entities.h"

namespace game {

void ent_player_t::turn() {
  const int32_t map_w = game.map_get().width;
  const int32_t map_h = game.map_get().height;
  librl::input_event_t event;
  if (game.input_event_pop(event)) {

    librl::int2 np = pos;

    switch (event.type) {
    case librl::input_event_t::key_up:    np.y > 0         ? --np.y : 0; break;
    case librl::input_event_t::key_down:  np.y < map_h - 1 ? ++np.y : 0; break;
    case librl::input_event_t::key_left:  np.x > 0         ? --np.x : 0; break;
    case librl::input_event_t::key_right: np.x < map_w - 1 ? ++np.x : 0; break;
    }

    if (np == pos) {
      // no move selected
      return;
    }

    // moving onto an entity which may count as an attack or pickup
    if (entity_t *ent = game.entity_find(np)) {
      interact_with(ent);
      turn_end();
      return;
    }

    // if moving to another floor tile
    if (game.map_get().get(np) == tile_floor) {
      pos = np;
      turn_end();
      return;
    }

    // not yet finished turn
    // ...
  }
}

void ent_player_t::interact_with(librl::entity_t *ent) {
  if (ent->type == ent_type_test) {
    attack(static_cast<librl::entity_actor_t*>(ent));
  }
}

void ent_test_t::turn() {
  const int32_t map_w = game.map_get().width;
  const int32_t map_h = game.map_get().height;

  librl::int2 np = pos;

  switch (librl::random(seed) & 3) {
  case 0: np.x < map_w - 1 ? ++np.x : 0; break;
  case 1: np.x > 0         ? --np.x : 0; break;
  case 2: np.y < map_h - 1 ? ++np.y : 0; break;
  case 3: np.y > 0         ? --np.y : 0; break;
  }

  if (entity_t *ent = game.entity_find(np)) {
    interact_with(ent);
    turn_end();
    return;
  }

  if (game.map_get().get(np) == tile_floor) {
    pos = np;
    turn_end();
    return;
  }

  // skip turn
  turn_end();
}

void ent_test_t::interact_with(entity_t *e) {
  // ... nothing for now
}

}  // namespace game
