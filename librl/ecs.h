#pragma once
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include "common.h"


namespace librl {

struct ecs_manager_t;

typedef uint32_t ecs_id_t;
typedef std::unordered_map<ecs_id_t, int32_t> ecs_refs_t;
typedef std::vector<ecs_id_t> ecs_dead_t;

enum {
  ecs_id_invalid = ~0u
};

template <typename type_t>
struct ecs_store_t {

  ~ecs_store_t() {
    clear();
  }

  void clear() {
    auto itt = map.begin();
    while (itt != map.end()) {
      delete itt->second;
      itt = std::next(itt);
    }
    map.clear();
  }

  type_t *get(ecs_id_t id) {
    auto itt = map.find(id);
    assert(itt != map.end());
    return itt->second;
  }

  type_t *try_get(ecs_id_t id) {
    auto itt = map.find(id);
    return (itt == map.end()) ? nullptr : itt->second;
  }

  type_t *insert(ecs_id_t id, type_t *inst) {
    assert(map.find(id) == map.end());
    return map[id] = inst;
  }

  void collect(const std::vector<ecs_id_t> &ids) {
    for (const ecs_id_t i : ids) {
      auto itt = map.find(i);
      if (itt != map.end()) {
        map.erase(itt);
      }
    }
  }

  const auto &get_all() const {
    return map;
  }

protected:
  std::unordered_map<ecs_id_t, type_t*> map;
};

struct ecs_manager_t {

  ecs_manager_t()
    : id_next(0) {
  }

  ecs_id_t id_new() {
    // create a new refcount for this entity
    assert(ref_count.find(id_next) == ref_count.end());
    ref_count[id_next] = 1;
    return id_next++;
  }

  void id_aquire(ecs_id_t id) {
    auto itt = ref_count.find(id);
    assert(itt != ref_count.end());
    ++itt->second;
  }

  void id_release(ecs_id_t id) {
    // note: might be decent to make id a ref so we can set it to invalid
    auto itt = ref_count.find(id);
    assert(itt != ref_count.end());
    assert(itt->second > 0);
    if (--itt->second == 0) {
      dead.push_back(itt->first);
      // remove from the ref_count list
      ref_count.erase(itt);
    }
  }

  const ecs_dead_t &dead_get() const {
    return dead;
  }

  void dead_clear() {
    dead.clear();
  }

protected:
  ecs_refs_t ref_count;
  ecs_dead_t dead;

  ecs_id_t id_next;
};

}  // namespace librl
