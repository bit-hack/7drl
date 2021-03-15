#include "game.h"
#include "entities.h"


using namespace librl;

namespace game {

ent_enemy_t::ent_enemy_t(uint32_t type, game::game_7drl_t &game)
  : entity_actor_t(type, game)
  , seed(game.random())
{
  accuracy = 0;
  damage = 0;
  defense = 0;
  evasion = 0;
  crit = 0;
  colour = 0xFF00FF;
  glyph = '£';
  sense = 250;
}

void ent_enemy_t::render() {
  auto &con = game.console_get();
  if (!game.player) {
    return;
  }
  if (librl::raycast(game.player->pos, pos, game.walls_get())) {
    con.attrib.get(pos.x, pos.y) = colour;
    con.chars.get(pos.x, pos.y) = glyph;
  }
}

void ent_vampire_t::on_give_damage(int32_t damage, entity_t *) {
  damage /= 2;
  game.message_post("%s leeched %d hp", name.c_str(), damage);
  hp += damage;
}

bool ent_wrath_t::turn() {
  using namespace librl;
  if (game.player) {
    const auto &pp = game.player->pos;
    if (raycast(pp, pos, game.walls_get())) {
      --timer;
    }
    if (timer <= 0) {
      timer = 2;
      const int32_t dx = int32_t(pp.x) - int32_t(pos.x);
      const int32_t dy = int32_t(pp.y) - int32_t(pos.y);
      const int32_t dist = dx * dx + dy * dy;
      if (dist > 5) {
        teleport_to_pos(pp);
        return true;
      }
    }
  }
  return ent_enemy_t::turn();
}


bool ent_dwarf_t::turn() {
  using namespace librl;
  if (game.player) {
    // head straight for the player if close enough
    const auto &pp = game.player->pos;
    const int32_t dx = int32_t(pp.x) - int32_t(pos.x);
    const int32_t dy = int32_t(pp.y) - int32_t(pos.y);
    const int32_t dist = dx * dx + dy * dy;
    if (dist >= 1 && dist <= 8) {
      const int2 d = {
        abs(dx) > abs(dy) ? sign(dx) : 0,
        abs(dx) > abs(dy) ? 0 : sign(dy)
      };
      const int2 np{ pos.x + d.x, pos.y + d.y };
      // smash the wall down
      if (game.walls_get().get(np)) {
        game.walls_get().clear(np);
        game.map_get().get(np) = '.';
      }
      if (entity_t *e = game.entity_find(np)) {
        interact_with(e);
        return true;
      }
      else {
        pos = np;
        return true;
      }
    }
  }
  return ent_enemy_t::turn();
}

bool ent_warlock_t::turn() {
  using namespace librl;
  if (!game.player) {
    return true;
  }
  const int2 &x = game.player->pos;
  if (raycast(x, pos, game.walls_get())) {
    if (timer-- == 0) {
      timer = spawn_time;
      spawn_skeleton();
    }
  }
  else {
    timer = spawn_time;
  }
  if (move_pfield(-1)) {
    return true;
  }
  if (move_random()) {
    return true;
  }
  // skip turn
  return true;
}

void entity_item_t::render() {
  auto &con = game.console_get();
  if (!game.player) {
    return;
  }
  if (librl::raycast(game.player->pos, pos, game.walls_get())) {
    con.attrib.get(pos.x, pos.y) = colour;
    con.chars.get(pos.x, pos.y) = glyph;
  }
}

void entity_equip_t::render() {
  auto &con = game.console_get();
  if (!game.player) {
    return;
  }
  if (librl::raycast(game.player->pos, pos, game.walls_get())) {
    con.attrib.get(pos.x, pos.y) = colour;
    con.chars.get(pos.x, pos.y) = glyph;
  }
}

void ent_stairs_t::use_on(entity_t *e) {
  if (e->is_type<ent_player_t>()) {
    game.message_post("%s proceeds deeper into the dungeon", e->name.c_str());
    game.map_next();
  }
}

void ent_gold_t::use_on(entity_t *e) {
  if (e->is_type<ent_player_t>()) {
    int value = int(15 + librl::random(seed, 15));
    game.message_post("%s gained %d gold", e->name.c_str(), value);
    static_cast<ent_player_t*>(e)->gold += value;
    game.entity_remove(this);
  }
}

void ent_potion_t::use_on(entity_t *e) {
  using namespace librl;

  entity_actor_t *act = e->as_a<entity_actor_t>();
  assert(act);

  const int32_t before = act->hp;
  act->hp = min<int32_t>(act->hp_max, act->hp + recovery);
  const int32_t diff = act->hp - before;
  game.message_post("%s used %s to recover %u hp", e->name.c_str(),
    name.c_str(), diff);
}

ent_gold_t::ent_gold_t(game::game_7drl_t &game)
  : entity_item_t(TYPE, game, /* can_pickup */ false)
  , seed(game.random())
{
  name = "gold";
  glyph = '$';
  colour = colour_gold;
}

void entity_actor_t::attack(entity_actor_t *target) {

  const auto rng = game.random() % 100;
  const auto crt = get_crit();

  const bool crit = rng < crt;
  if (crit) {
    const uint32_t damage = get_damage();
    target->hp -= damage;
    const bool dead = target->hp <= 0;
    on_give_damage(damage, target);        // callback
    target->on_take_damage(damage, this);  // callback
    game.message_post("%s was critically hit by %s for %u damage%s",
      target->name.c_str(), name.c_str(), damage, dead ? " and killed" : "");
  }
  else {
    const int32_t chance_to_hit = get_accuracy() - target->get_evasion();
    const bool miss = (game.random() % 100) < chance_to_hit;
    if (miss) {
      game.message_post("%s missed %s", name.c_str(), target->name.c_str());
      return;
    }
    const int32_t damage = librl::clamp<int32_t>(0, get_damage() - target->get_defense(), 1000);
    target->hp -= damage;
    on_give_damage(damage, target);        // callback
    target->on_take_damage(damage, this);  // callback
    const bool dead = target->hp <= 0;
    game.message_post("%s was hit by %s for %u damage%s",
      target->name.c_str(), name.c_str(), damage, dead ? " and killed" : "");
  }
  if (target->hp <= 0) {
    target->kill();
  }
}

void entity_actor_t::kill() {
  if (this == game.player) {
    game.player = nullptr;
    // XXX: need to make sure we register the end game
  }
  game.entity_remove(this);
}

void entity_item_t::picked_up(entity_t *by) {
  assert(by);
  game.message_post("%s picked up a %s", by->name.c_str(), name.c_str());
  // remove from the game world when put in inventory
  game.entity_remove(this);
}

void entity_equip_t::picked_up(entity_t *by) {
  assert(by);
  game.message_post("%s picked up a %s", by->name.c_str(), name.c_str());
  // remove from the game world when put in inventory
  game.entity_remove(this);
}

ent_player_t::ent_player_t(game::game_7drl_t &game)
  : entity_actor_t(TYPE, game)
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

void ent_enemy_t::interact_with(entity_t *e) {
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

void ent_player_t::interact_with(entity_t *ent) {
  assert(ent);
  // attacking
  const bool is_enemy =
    ent->is_subclass<entity_actor_t>() && !ent->is_type<ent_player_t>();
  if (is_enemy) {
    attack(static_cast<entity_actor_t*>(ent));
    return;
  }
  // equipables
  if (ent->is_subclass<entity_equip_t>()) {
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
  if (ent->is_subclass<entity_item_t>()) {
    entity_item_t *item = static_cast<entity_item_t*>(ent);
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

void ent_player_t::render() {
  auto &con = game.console_get();
  con.attrib.get(pos.x, pos.y) = colour_player;
  con.chars.get(pos.x, pos.y) = '@';
}

}  // namespace game
