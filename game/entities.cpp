#include "game.h"
#include "entities.h"

namespace game {

ent_player_t::ent_player_t(game::game_t &game)
  : librl::entity_actor_t(TYPE, game)
  , user_dir{ 0, 0 }
{
  hp_max = 150;
  name = "hero";
  hp = 100;
  gold = 0;
  xp = 0;
}

void ent_player_t::_enumerate(librl::gc_enum_t &func) {
  inventory._enumerate(func);
}

void ent_player_t::kill() {
  hp = 0;
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

void ent_enemy_t::interact_with(librl::entity_t *e) {
  switch (e->type) {
  case ent_type_player:
    attack(static_cast<entity_actor_t*>(e));
    break;
  }
}

bool ent_enemy_t::move_random() {
  using namespace librl;
  int2 np = pos;
  if (librl::random(seed, 3) == 0) {
    switch (librl::random(seed, 4)) {
    case 0: ++np.x; break;
    case 1: --np.x; break;
    case 2: ++np.y; break;
    case 3: --np.y; break;
    }
  }
  if (!game.walls_get().get(np)) {
    pos = np;
    return true;
  }
  if (entity_t *ent = game.entity_find(np)) {
    return false;
  }
  return false;
}

bool ent_enemy_t::move_pfield(int dir) {
  using namespace librl;
  librl::int2 np = pos;
  int32_t dx = 0;
  int32_t dy = 0;
  game.pfield_get().diff(pos.x, pos.y, dx, dy, sense);
  if (dx != 0 || dy != 0) {
    if (librl::abs(dx) > librl::abs(dy)) {
      np.x += dx * dir;
    }
    else {
      np.y += dy * dir;
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
  return false;
}

bool ent_enemy_t::turn() {
  if (move_pfield()) {
    return true;
  }
  if (move_random()) {
    return true;
  }
  // skip turn
  return true;
}

void ent_player_t::interact_with(librl::entity_t *ent) {
  assert(ent);
  // attacking
  const bool is_enemy =
    ent->is_subclass<entity_actor_t>() && !ent->is_type<ent_player_t>();
  if (is_enemy) {
    attack(static_cast<librl::entity_actor_t*>(ent));
    return;
  }
  // equipables
  if (ent->is_subclass<librl::entity_equip_t>()) {
    if (inventory.add(ent)) {
      game.message_post("%s picked up %s", name.c_str(), ent->name.c_str());
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
        game.message_post("%s picked up %s", name.c_str(), ent->name.c_str());
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

void ent_warlock_t::spawn_skeleton() {
  using namespace librl;
  for (int32_t i = 0; i < 4; ++i) {
    int2 p = { pos.x + int32_t(random(seed, 5)) - 2,
               pos.y + int32_t(random(seed, 5)) - 2 };

    if (p.x <= 0 ||
        p.y <= 0 ||
        p.x >= int32_t(game.map_get().width  - 1) ||
        p.y >= int32_t(game.map_get().height - 1)) {
      continue;
    }

    if (!game.walls_get().get(p)) {
      game.message_post("%s spawned a skeleton", name.c_str());
      entity_t *e = game.entity_add(game.gc.alloc<game::ent_skeleton_t>(game));
      e->pos = p;
      break;
    }
  }
}

void ent_wrath_t::teleport_to_pos(const librl::int2 &j) {
  using namespace librl;
  for (int32_t i = 0; i < 4; ++i) {
    int2 p = { j.x + int32_t(random(seed, 5)) - 2,
               j.y + int32_t(random(seed, 5)) - 2 };

    if (p.x <= 0 ||
        p.y <= 0 ||
        p.x >= int32_t(game.map_get().width - 1) ||
        p.y >= int32_t(game.map_get().height - 1)) {
      continue;
    }

    if (!game.walls_get().get(p)) {
      game.message_post("%s teleported", name.c_str());
      pos = p;
      break;
    }
  }
}

void ent_player_t::on_give_damage(int32_t damage, entity_t *to) {
  if (entity_actor_t *act = to->as_a<entity_actor_t>()) {
    if (act->hp > 0) {
      return;
    }
    int gain = 0;
    if (to->is_type<ent_goblin_t>()) {
      gain = 10;
    }
    if (to->is_type<ent_ogre_t>()) {
      gain = 20;
    }
    if (to->is_type<ent_dwarf_t>()) {
      gain = 30;
    }
    if (to->is_type<ent_mimic_t>()) {
      gain = 75;
    }
    if (to->is_type<ent_skeleton_t>()) {
      gain = 77;
    }
    if (to->is_type<ent_vampire_t>()) {
      gain = 75;
    }
    if (to->is_type<ent_wrath_t>()) {
      gain = 80;
    }
    if (to->is_type<ent_warlock_t>()) {
      gain = 150;
    }
    if (gain) {
      xp += gain;
      game.message_post("%s gained %dxp", name.c_str(), gain);
    }
  }
}

}  // namespace game
