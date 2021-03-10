#include "game.h"
#include "entities.h"

namespace game {

ent_player_t::ent_player_t(librl::game_t &game)
  : librl::entity_actor_t(TYPE, game)
  , user_dir{ 0, 0 }
{
  name = "hero";
  hp = 100;
  gold = 0;
}

void ent_player_t::_enumerate(librl::gc_enum_t &func) {
  inventory._enumerate(func);
}

bool ent_player_t::turn() {
  const librl::int2 np{ pos.x + user_dir.x, pos.y + user_dir.y };
  if (np == pos) {
    // no move selected
    return false;
  }
  // moving onto an entity which may count as an attack or pickup
  if (entity_t *ent = game.entity_find(np)) {
    interact_with(ent);
    return true;
  }
  // we can move if its not a wall
  if (!game.walls_get().get(np)) {
    pos = np;
    return true;
  }
  // not yet finished turn
  // ...
  return false;
}

void ent_player_t::interact_with(librl::entity_t *ent) {
  assert(ent);
  // attaching
  const bool is_enemy = ent->is_type<ent_goblin_t>();
  if (is_enemy) {
    attack(static_cast<librl::entity_actor_t*>(ent));
    return;
  }
  // equipables
  if (ent->is_subclass<librl::entity_equip_t>()) {
    if (inventory.add(ent)) {
      // remove from the game world
      game.entity_remove(ent);
    }
    else {
      game.message_post("%s's inventory full", name.c_str());
    }
  }
  // items
  if (ent->is_subclass<librl::entity_item_t>()) {
    librl::entity_item_t *item = static_cast<librl::entity_item_t*>(ent);
    if (item->can_pickup) {
      if (inventory.add(ent)) {
        static_cast<librl::entity_item_t*>(ent)->picked_up(this);
        // remove from the game world
        game.entity_remove(ent);
      }
      else {
        game.message_post("%s's inventory full", name.c_str());
      }
    }
    else {
      // use right now
      item->use_on(this);
    }
  }
}

bool ent_goblin_t::turn() {
  librl::int2 np = pos;

  int32_t dx = 0;
  int32_t dy = 0;
  game.pfield_get().diff(pos.x, pos.y, dx, dy);
  if (dx != 0 || dy != 0) {
    if (librl::abs(dx) > librl::abs(dy)) {
      np.x += dx;
    }
    else {
      np.y += dy;
    }
  }
  else {
    if (librl::random(seed, 3) == 0) {
      switch (librl::random(seed) & 3) {
      case 0: ++np.x; break;
      case 1: --np.x; break;
      case 2: ++np.y; break;
      case 3: --np.y; break;
      }
    }
  }

  if (entity_t *ent = game.entity_find(np)) {
    interact_with(ent);
    return true;
  }

  if (!game.walls_get().get(np)) {
    pos = np;
    return true;
  }

  // skip turn
  return true;
}

void ent_goblin_t::interact_with(entity_t *e) {
  switch (e->type) {
  case ent_type_player:
    attack(static_cast<entity_actor_t*>(e));
    break;
  }
}

}  // namespace game
