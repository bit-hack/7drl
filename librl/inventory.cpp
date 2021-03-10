#include "entity.h"
#include "inventory.h"

namespace librl {

void inventory_t::_enumerate(gc_enum_t &func) {
  for (entity_t *e : slot) {
    if (e) {
      func(e);
    }
  }
}

void inventory_t::item_use(size_t i, entity_t *target) {
  // validate the item
  assert(i >= 0 && i < slot.size());
  entity_t *e = slot[i];
  if (!e) {
    return;
  }
  assert(e->is_subclass<entity_item_t>());
  entity_item_t *item = static_cast<entity_item_t*>(e);
  // use the item
  item->use_on(target);
  // remove item from inventory
  slot[i] = nullptr;
}

bool inventory_t::item_add(entity_t *e) {
  for (size_t i = 0; i < slot.size(); ++i) {
    if (slot[i] == nullptr) {
      slot[i] = e;
      return true;
    }
  }
  return false;
}

}  // namespace librl
