#include "entity.h"
#include "game.h"

namespace librl {

void entity_actor_t::attack(entity_actor_t *target) {
  const bool crit = (game.random() % 100) < get_crit();
  if (crit) {
    const uint32_t damage = get_damage();
    target->hp -= damage;
    const bool dead = target->hp <= 0;
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

} // namespace librl
