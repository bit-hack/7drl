#include "entities.h"
#include "inventory.h"


namespace game {

void inventory_t::_enumerate(librl::gc_enum_t &func) {
  for (entity_t *e : slot) {
    if (e) {
      func(e);
    }
  }
}

void inventory_t::use(size_t i, entity_t *target) {
  // validate the item
  assert(i >= 0 && i < slot.size());
  entity_t *e = slot[i];
  if (!e) {
    return;
  }
  // use the item
  if (e->is_subclass<entity_item_t>()) {
    entity_item_t *item = static_cast<entity_item_t*>(e);
    // use the item
    item->use_on(target);
    // remove item from inventory
    slot[i] = nullptr;
  }
  // equip the item
  if (e->is_subclass<entity_equip_t>()) {
    entity_equip_t *item = static_cast<entity_equip_t*>(e);
    // do some rotations to equip to first slot
    for (size_t k = i; k >= 1; --k) {
      slot[k] = slot[k - 1];
    }
    slot[0] = e;
  }
}

bool inventory_t::add(entity_t *e) {
  for (size_t i = 0; i < slot.size(); ++i) {
    if (slot[i] == nullptr) {
      slot[i] = e;
      return true;
    }
  }
  return false;
}

entity_equip_t *inventory_t::get_equip(uint32_t i) const {
  if (entity_t *e = slot[i]) {
    if (e->is_subclass<entity_equip_t>()) {
      return static_cast<entity_equip_t*>(e);
    }
  }
  return nullptr;
}

int inventory_t::get_damage() const {
  int out = 0;
  if (entity_equip_t *e = get_equip(0)) {
    out += e->damage;
  }
  if (entity_equip_t *e = get_equip(1)) {
    out += e->damage;
  }
  return out;
}

int inventory_t::get_accuracy() const {
  int out = 0;
  if (entity_equip_t *e = get_equip(0)) {
    out += e->accuracy;
  }
  if (entity_equip_t *e = get_equip(1)) {
    out += e->accuracy;
  }
  return out;
}

int inventory_t::get_evasion() const {
  int out = 0;
  if (entity_equip_t *e = get_equip(0)) {
    out += e->evasion;
  }
  if (entity_equip_t *e = get_equip(1)) {
    out += e->evasion;
  }
  return out;
}

int inventory_t::get_defense() const {
  int out = 0;
  if (entity_equip_t *e = get_equip(0)) {
    out += e->defense;
  }
  if (entity_equip_t *e = get_equip(1)) {
    out += e->defense;
  }
  return out;
}

int inventory_t::get_crit() const {
  int out = 0;
  if (entity_equip_t *e = get_equip(0)) {
    out += e->crit;
  }
  if (entity_equip_t *e = get_equip(1)) {
    out += e->crit;
  }
  return out;
}

}  // namespace librl
