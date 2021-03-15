#pragma once
#include <array>

// librl
#include "gc.h"


namespace game {

struct entity_t;
struct entity_equip_t;

struct inventory_t : public librl::gc_base_t {

  static const size_t num_slots = 8;

  inventory_t() {
    slot.fill(nullptr);
  }

  void _enumerate(librl::gc_enum_t &func) override;

  bool add(entity_t *e);

  void use(size_t i, entity_t *target);

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

  int32_t get_damage() const;
  int32_t get_accuracy() const;
  int32_t get_evasion() const;
  int32_t get_defense() const;
  int32_t get_crit() const;

protected:

  entity_equip_t *get_equip(uint32_t slot) const;

  std::array<entity_t *, num_slots> slot;
};

}  // namespace librl
