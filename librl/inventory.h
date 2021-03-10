#pragma once
#include <array>

#include "gc.h"


namespace librl {

struct entity_t;

struct inventory_t : public gc_base_t {

  static const size_t num_slots = 8;

  inventory_t() {
    slot.fill(nullptr);
  }

  void _enumerate(gc_enum_t &func) override;

  bool item_add(entity_t *e);

  void item_use(size_t i, entity_t *target);

  void drop(size_t i) {
    assert(i >= 0 && i < slot.size());
    slot[i] = nullptr;
  }

  const auto &slots() const {
    return slot;
  }

  void clear() {
    slot.fill(nullptr);
  }

protected:
  std::array<entity_t *, num_slots> slot;
};

}  // namespace librl
